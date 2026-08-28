/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "controller/usb_controller.hpp"

#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <format>

#include <logmich/log.hpp>
#include <unsebu/usb_helper.hpp>

#include "controller_message.hpp"
#include "raise_exception.hpp"
#include "xboxmsg.hpp"

namespace xboxdrv {

USBController::USBController(libusb_device* dev) :
  m_dev(dev),
  m_handle(nullptr),
  m_transfers(),
  m_interfaces(),
  m_detached_interfaces(),
  m_shutting_down(false),
  m_usbpath(),
  m_usbid(),
  m_name()
{
  // Own a ref so shared devices (e.g. wireless receiver → 4 slots) and the
  // daemon's usb_find_device_by_path() result are released safely.
  libusb_ref_device(m_dev);

  int ret = libusb_open(dev, &m_handle);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_open() failed: " << libusb_strerror(ret));
  }
  else
  {
    // get usbpath, usbid and name
    m_usbpath = std::format("{:03d}:{:03d}",
                            static_cast<int>(libusb_get_bus_number(dev)),
                            static_cast<int>(libusb_get_device_address(dev)));

    libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(dev, &desc);
    if (ret == LIBUSB_SUCCESS)
    {
      m_usbid = std::format("{:04x}:{:04x}",
                            static_cast<int>(desc.idVendor),
                            static_cast<int>(desc.idProduct));

      char buf[1024];
      int len;
      if (false)
      { // FIXME: do we need the manufacturer name?
        len = libusb_get_string_descriptor_ascii(m_handle, desc.iManufacturer,
                                                 reinterpret_cast<unsigned char*>(buf), sizeof(buf));
        if (len > 0)
        {
          m_name.append(buf, len);
          m_name.append(" ");
        }
      }

      len = libusb_get_string_descriptor_ascii(m_handle, desc.iProduct,
                                               reinterpret_cast<unsigned char*>(buf), sizeof(buf));
      if (len > 0)
      {
        m_name.append(buf, len);
      }
    }
  }
}

USBController::~USBController()
{
  m_shutting_down = true;

  // Cancel outstanding transfers. Completion callbacks remove them from
  // m_transfers; do not free here (libusb owns the transfer until the
  // callback runs or cancel fails).
  for (libusb_transfer* transfer : m_transfers)
  {
    int ret = libusb_cancel_transfer(transfer);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND)
    {
      log_error("libusb_cancel_transfer() failed: {}", libusb_strerror(ret));
    }
  }

  // Drain cancellations with a timeout so a dead device cannot hang
  // the process forever (see GitHub #239 / historical libusb teardown crashes).
  const int max_iterations = 100;
  for (int i = 0; !m_transfers.empty() && i < max_iterations; ++i)
  {
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000; // 10ms
    int ret = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_INTERRUPTED)
    {
      log_error("libusb_handle_events_timeout_completed() failure: {}", libusb_strerror(ret));
      break;
    }
  }

  if (!m_transfers.empty())
  {
    log_error("USBController teardown: {} transfer(s) still pending after cancel; "
              "abandoning (device likely already gone)", m_transfers.size());
    // Do not libusb_free_transfer here: the transfer may still be referenced
    // inside libusb. Abandon the set; process exit or libusb_close will drop them.
    m_transfers.clear();
  }

  for (int ifnum : m_interfaces)
  {
    int ret = libusb_release_interface(m_handle, ifnum);
    if (ret != LIBUSB_SUCCESS)
    {
      log_debug("libusb_release_interface({}) failed: {}", ifnum, libusb_strerror(ret));
    }
    // Hand the interface back to the kernel driver (e.g. xpad) when we
    // detached it with --detach-kernel-driver. Must run after release.
    if (m_handle && m_detached_interfaces.count(ifnum))
    {
      ret = libusb_attach_kernel_driver(m_handle, ifnum);
      if (ret == LIBUSB_SUCCESS)
      {
        log_info("reattached kernel driver on interface {}", ifnum);
      }
      else if (ret != LIBUSB_ERROR_NOT_FOUND &&
               ret != LIBUSB_ERROR_NO_DEVICE &&
               ret != LIBUSB_ERROR_NOT_SUPPORTED)
      {
        log_debug("libusb_attach_kernel_driver({}): {}", ifnum, libusb_strerror(ret));
      }
    }
  }
  m_interfaces.clear();
  m_detached_interfaces.clear();

  if (m_handle)
  {
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  if (m_dev)
  {
    libusb_unref_device(m_dev);
    m_dev = nullptr;
  }
}

std::string
USBController::get_usbpath() const
{
  return m_usbpath;
}

std::string
USBController::get_usbid() const
{
  return m_usbid;
}

std::string
USBController::get_name() const
{
  return m_name;
}

bool
USBController::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  // Dummy default for the pure-virtual-in-destructor case (PR #220):
  // on_read_data can still run while ~USBController drains cancellations
  // after the derived class has already been destroyed.
  (void)data;
  (void)len;
  (void)msg_out;
  return false;
}

void
USBController::usb_submit_read(int endpoint, int len)
{
  if (m_shutting_down || is_disconnected() || !m_handle)
  {
    log_debug("usb_submit_read skipped: controller shutting down or disconnected");
    return;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_IN),
                                 data, len,
                                 &USBController::on_read_data_wrap, this,
                                 0); // timeout
  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << libusb_strerror(ret));
  }
  else
  {
    m_transfers.insert(transfer);
  }
}

void
USBController::usb_write(int endpoint, uint8_t* data_in, int len)
{
  // Soft-fail after unplug so set_led/set_rumble during shutdown do not throw.
  if (m_shutting_down || is_disconnected() || !m_handle)
  {
    log_debug("usb_write skipped: controller shutting down or disconnected");
    return;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  memcpy(data, data_in, len);

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_OUT),
                                 data, len,
                                 &USBController::on_write_data_wrap, this,
                                 0); // timeout

  int ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    if (ret == LIBUSB_ERROR_NO_DEVICE)
    {
      send_disconnect();
      return;
    }
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << libusb_strerror(ret));
  }
  else
  {
    m_transfers.insert(transfer);
  }
}

void
USBController::usb_control(uint8_t  bmRequestType, uint8_t  bRequest,
                           uint16_t wValue, uint16_t wIndex,
                           uint8_t* data_in, uint16_t wLength)
{
  if (m_shutting_down || is_disconnected() || !m_handle)
  {
    log_debug("usb_control skipped: controller shutting down or disconnected");
    return;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  // create and fill control buffer
  uint8_t* data = static_cast<uint8_t*>(malloc(wLength + 8));
  libusb_fill_control_setup(data, bmRequestType, bRequest, wValue, wIndex, wLength);
  memcpy(data + 8, data_in, wLength);
  libusb_fill_control_transfer(transfer, m_handle, data,
                               &USBController::on_control_wrap, this,
                               0);

  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    if (ret == LIBUSB_ERROR_NO_DEVICE)
    {
      send_disconnect();
      return;
    }
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << libusb_strerror(ret));
  }
  else
  {
    m_transfers.insert(transfer);
  }
}


void
USBController::on_control(libusb_transfer* transfer)
{
  log_debug("control transfer status={}", libusb_error_name(transfer->status));

  if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE && !m_shutting_down)
  {
    send_disconnect();
  }

  m_transfers.erase(transfer);
  libusb_free_transfer(transfer);
}

void
USBController::on_write_data(libusb_transfer* transfer)
{
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED ||
      transfer->status == LIBUSB_TRANSFER_CANCELLED)
  {
    // ok
  }
  else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
  {
    if (!m_shutting_down)
    {
      send_disconnect();
    }
  }
  else
  {
    log_error("USB write failure: status={} actual_length={} length={}",
              libusb_error_name(transfer->status), transfer->actual_length, transfer->length);
  }

  m_transfers.erase(transfer);
  libusb_free_transfer(transfer);
}

void
USBController::on_read_data(libusb_transfer* transfer)
{
  assert(transfer);

  if (transfer->status == LIBUSB_TRANSFER_COMPLETED)
  {
    if (m_shutting_down || is_disconnected())
    {
      m_transfers.erase(transfer);
      libusb_free_transfer(transfer);
      return;
    }

    ControllerMessage msg;
    if (parse(transfer->buffer, transfer->actual_length, &msg))
    {
      submit_msg(msg, m_message_descriptor);
    }

    int ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS)
    {
      log_error("failed to resubmit USB transfer: {}", libusb_strerror(ret));
      m_transfers.erase(transfer);
      libusb_free_transfer(transfer);
      send_disconnect();
    }
  }
  else if (transfer->status == LIBUSB_TRANSFER_CANCELLED)
  {
    m_transfers.erase(transfer);
    libusb_free_transfer(transfer);
  }
  else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
  {
    m_transfers.erase(transfer);
    libusb_free_transfer(transfer);
    if (!m_shutting_down)
    {
      send_disconnect();
    }
  }
  else
  {
    // STALL / ERROR / TIMED_OUT: the continuous read loop is dead; treat as disconnect
    // so the slot is freed (GitHub #239) instead of leaving a zombie controller.
    log_error("USB read failure: status={} actual_length={} length={}",
              libusb_error_name(transfer->status), transfer->actual_length, transfer->length);
    m_transfers.erase(transfer);
    libusb_free_transfer(transfer);
    if (!m_shutting_down)
    {
      send_disconnect();
    }
  }
}

void
USBController::usb_claim_interface(int ifnum, bool try_detach)
{
  // keep track of all claimed interfaces so they can be released in
  // the destructor
  assert(m_interfaces.find(ifnum) == m_interfaces.end());
  m_interfaces.insert(ifnum);

  // Remember if a kernel driver was bound so we can reattach on exit.
  bool kernel_driver_was_active = false;
  if (try_detach)
  {
    int active = libusb_kernel_driver_active(m_handle, ifnum);
    kernel_driver_was_active = (active == 1);
  }

  int err = unsebu::usb_claim_n_detach_interface(m_handle, ifnum, try_detach);
  if (err != 0)
  {
    std::ostringstream out;
    out << " Error couldn't claim the USB interface: " << libusb_strerror(err) << std::endl
        << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
    throw std::runtime_error(out.str());
  }

  if (kernel_driver_was_active)
  {
    m_detached_interfaces.insert(ifnum);
  }
}

int
USBController::usb_find_ep(int direction, uint8_t if_class, uint8_t if_subclass, uint8_t if_protocol)
{
  libusb_config_descriptor* config;
  int ret = libusb_get_config_descriptor(m_dev, 0 /* config_index */, &config);

  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << libusb_strerror(ret));
  }
  else
  {
    int ret_endpoint = -1;

    // FIXME: no need to search all interfaces, could just check the one we acutally use
    for(libusb_interface const* interface = config->interface;
        interface != config->interface + config->bNumInterfaces;
        ++interface)
    {
      for(libusb_interface_descriptor const* altsetting = interface->altsetting;
          altsetting != interface->altsetting + interface->num_altsetting;
          ++altsetting)
      {
        log_debug("Interface: {}", static_cast<int>(altsetting->bInterfaceNumber));

        for(libusb_endpoint_descriptor const* endpoint = altsetting->endpoint;
            endpoint != altsetting->endpoint + altsetting->bNumEndpoints;
            ++endpoint)
        {
          log_debug("    Endpoint: {} ({})",
                    int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK),
                    ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? "IN" : "OUT"));

          if ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == direction &&
              altsetting->bInterfaceClass    == if_class    &&
              altsetting->bInterfaceSubClass == if_subclass &&
              altsetting->bInterfaceProtocol == if_protocol)
          {
            ret_endpoint = static_cast<int>(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK);
          }
        }
      }
    }
    libusb_free_config_descriptor(config);

    if (ret_endpoint < 0)
    {
      raise_exception(std::runtime_error, "couldn't find matching endpoint");
    }
    else
    {
      return ret_endpoint;
    }
  }
}

} // namespace xboxdrv

/* EOF */
