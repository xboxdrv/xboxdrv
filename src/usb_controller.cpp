/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "usb_controller.hpp"

#include <boost/format.hpp>

#include "log.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

USBController::USBController(libusb_device* dev) :
  m_dev(dev),
  m_handle(0),
  m_transfers(),
  m_interfaces(),
  m_usbpath(),
  m_usbid(),
  m_name()
{
  int ret = libusb_open(dev, &m_handle);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_open() failed: " << usb_strerror(ret));
  }
  else
  {
    // get usbpath, usbid and name
    m_usbpath = (boost::format("%03d:%03d")
                 % static_cast<int>(libusb_get_bus_number(dev))
                 % static_cast<int>(libusb_get_device_address(dev))).str();

    libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(dev, &desc);
    if (ret == LIBUSB_SUCCESS)
    {
      m_usbid = (boost::format("%04x:%04x")
                 % static_cast<int>(desc.idVendor)
                 % static_cast<int>(desc.idProduct)).str();

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
  m_is_disconnected = true;

  // cancel all transfers
  for(std::set<libusb_transfer*>::iterator it = m_transfers.begin(); it != m_transfers.end(); ++it)
  {
    libusb_cancel_transfer(*it);
  }

  struct timeval to;
  to.tv_sec = 1;
  to.tv_usec = 0;

  // wait for cancel to succeed
  while (!m_transfers.empty())
  {
    int ret = libusb_handle_events_timeout_completed(NULL, &to, NULL);
    if (ret != 0)
    {
      log_error("libusb_handle_events_timeout_completed() failure: " << ret);
    }
  }

  // release all claimed interfaces
  for(std::set<int>::iterator it = m_interfaces.begin(); it != m_interfaces.end(); ++it)
  {
    libusb_release_interface(m_handle, *it);
  }

  // read and write transfers might still be going on and might need to be canceled
  libusb_close(m_handle);
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
USBController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  // dummy method for destructor
  return false;
}

void
USBController::usb_submit_read(int endpoint, int len)
{
  libusb_transfer* transfer = libusb_alloc_transfer(0);

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 endpoint | LIBUSB_ENDPOINT_IN,
                                 data, len,
                                 &USBController::on_read_data_wrap, this,
                                 0); // timeout
  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
  else
  {
    m_transfers.insert(transfer);
  }
}

void
USBController::usb_write(int endpoint, uint8_t* data_in, int len)
{
  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  // copy data into a newly allocated buffer
  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  memcpy(data, data_in, len);

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 endpoint | LIBUSB_ENDPOINT_OUT,
                                 data, len,
                                 &USBController::on_write_data_wrap, this,
                                 0); // timeout

  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
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
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
  else
  {
    m_transfers.insert(transfer);
  }
}

void
USBController::on_control(libusb_transfer* transfer)
{
  log_debug("control transfer");

  m_transfers.erase(transfer);
  libusb_free_transfer(transfer);
}

void
USBController::on_write_data(libusb_transfer* transfer)
{
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED)
  {
    // ok
  }
  else if (transfer->status == LIBUSB_TRANSFER_CANCELLED)
  {
    // ok
  }
  else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
  {
    send_disconnect();
  }
  else
  {
    log_error("USB write failure: " << transfer->length << ": " << usb_transfer_strerror(transfer->status));
  }

  m_transfers.erase(transfer);
  libusb_free_transfer(transfer);
}

void
USBController::on_read_data(libusb_transfer* transfer)
{
  assert(transfer);

  switch(transfer->status)
  {
    case LIBUSB_TRANSFER_COMPLETED:
      // process data
      XboxGenericMsg msg;
      if (parse(transfer->buffer, transfer->actual_length, &msg))
      {
        submit_msg(msg);
      }

      if (m_is_disconnected)
      {
        m_transfers.erase(transfer);
        libusb_free_transfer(transfer);
      }
      else
      {
        int ret;
        ret = libusb_submit_transfer(transfer);
        if (ret != LIBUSB_SUCCESS) // could also check for LIBUSB_ERROR_NO_DEVICE
        {
          log_error("failed to resubmit USB transfer: " << usb_strerror(ret));
          m_transfers.erase(transfer);
          libusb_free_transfer(transfer);
          send_disconnect();
        }
      }
      break;

    case LIBUSB_TRANSFER_CANCELLED:
      m_transfers.erase(transfer);
      libusb_free_transfer(transfer);
      break;

    case LIBUSB_TRANSFER_NO_DEVICE:
      m_transfers.erase(transfer);
      libusb_free_transfer(transfer);
      send_disconnect();
      break;

    default:
      log_error("USB read failure: " << transfer->length << ": " << usb_transfer_strerror(transfer->status));
      m_transfers.erase(transfer);
      libusb_free_transfer(transfer);
      break;
  }
}

void
USBController::usb_claim_interface(int ifnum, bool try_detach)
{
  // keep track of all claimed interfaces so they can be released in
  // the destructor
  assert(m_interfaces.find(ifnum) == m_interfaces.end());
  m_interfaces.insert(ifnum);

  int err = usb_claim_n_detach_interface(m_handle, ifnum, try_detach);
  if (err != 0)
  {
    std::ostringstream out;
    out << " Error couldn't claim the USB interface: " << usb_strerror(err) << std::endl
        << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
    throw std::runtime_error(out.str());
  }
}

int
USBController::usb_find_ep(int direction, uint8_t if_class, uint8_t if_subclass, uint8_t if_protocol)
{
  libusb_config_descriptor* config;
  int ret = libusb_get_config_descriptor(m_dev, 0 /* config_index */, &config);

  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << usb_strerror(ret));
  }
  else
  {
    int ret_endpoint = -1;

    // FIXME: no need to search all interfaces, could just check the one we acutally use
    for(const libusb_interface* interface = config->interface;
        interface != config->interface + config->bNumInterfaces;
        ++interface)
    {
      for(const libusb_interface_descriptor* altsetting = interface->altsetting;
          altsetting != interface->altsetting + interface->num_altsetting;
          ++altsetting)
      {
        log_debug("Interface: " << static_cast<int>(altsetting->bInterfaceNumber));

        for(const libusb_endpoint_descriptor* endpoint = altsetting->endpoint;
            endpoint != altsetting->endpoint + altsetting->bNumEndpoints;
            ++endpoint)
        {
          log_debug("    Endpoint: " << int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK) <<
                    "(" << ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? "IN" : "OUT") << ")");

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

/* EOF */
