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

#include "usb_helper.hpp"
#include "raise_exception.hpp"

USBController::USBController(libusb_device* dev) :
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
      ret = libusb_get_string_descriptor_ascii(m_handle, desc.iManufacturer,
                                               reinterpret_cast<unsigned char*>(buf), sizeof(buf));
      if (ret == LIBUSB_SUCCESS)
      {
        m_name = buf;
      }

      ret = libusb_get_string_descriptor_ascii(m_handle, desc.iProduct, 
                                               reinterpret_cast<unsigned char*>(buf), sizeof(buf));
      if (ret == LIBUSB_SUCCESS)
      {
        m_name += " ";
        m_name += buf;
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
USBController::claim_interface(int ifnum, bool try_detach)
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
USBController::release_interface(int ifnum)
{
  // should be called before closing the device handle
  libusb_release_interface(m_handle, ifnum); 
}

/* EOF */
