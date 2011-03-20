/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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
  //libusb_release_interface(m_handle, 0); 
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

void
USBController::usb_submit_read(int endpoint, int len)
{
  log_debug("ep: " << endpoint << " len: " << len);

  libusb_transfer* transfer = libusb_alloc_transfer(0);

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  //FIXME: transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 endpoint | LIBUSB_ENDPOINT_IN,
                                 data, len,
                                 &USBController::on_read_data_wrap, this,
                                 0); // timeout
  int ret;
  ret = libusb_submit_transfer(transfer);
  log_debug("libusb_submit_transfer: " << usb_strerror(ret));
}

void
USBController::usb_cancel_read()
{
  assert(!"implement me");
}

void
USBController::on_read_data(libusb_transfer *transfer)
{
  log_trace();

  assert(transfer);

  // process data
  XboxGenericMsg msg;
  if (parse(transfer->buffer, transfer->actual_length, &msg))
  {
    submit_msg(msg);
  }

  if (false) // cleanup
  {
    libusb_free_transfer(transfer);
  }
  else // resubmit
  {   
    int ret;
    ret = libusb_submit_transfer(transfer);
    assert(ret == LIBUSB_SUCCESS); // FIXME
  }
}

void
USBController::usb_claim_interface(int ifnum, bool try_detach)
{
  int err = usb_claim_n_detach_interface(m_handle, ifnum, try_detach);
  if (err != 0) 
  {
    std::ostringstream out;
    out << " Error couldn't claim the USB interface: " << usb_strerror(err) << std::endl
        << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
    throw std::runtime_error(out.str());
  }
}

void
USBController::usb_release_interface(int ifnum)
{
  // should be called before closing the device handle
  libusb_release_interface(m_handle, ifnum); 
}

void
USBController::usb_write(int endpoint, uint8_t* data, int len)
{
  log_error("not implemented");
#ifdef FIXME
  int transferred = 0;
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_OUT | endpoint,
                                      data, len, &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }

  if (transferred != len)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() short write: "
                    << len << " - " << transferred);
  }
#endif
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
