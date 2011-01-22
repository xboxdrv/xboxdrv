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

#include "playstation3_usb_controller.hpp"

#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>

#include "xboxmsg.hpp"
#include "usb_helper.hpp"

Playstation3USBController::Playstation3USBController(libusb_device* dev, bool try_detach) :
  m_handle(0),
  endpoint_in(1),
  endpoint_out(2)
{
  int ret = libusb_open(dev, &m_handle);
  if (ret != LIBUSB_SUCCESS)
  {
    std::ostringstream out;
    out << "Playstation3USBController: libusb_open() error: " << usb_strerror(ret);
    throw std::runtime_error(out.str());
  }
  else
  {
    int err = usb_claim_n_detach_interface(m_handle, 0, try_detach);
    if (err != 0)
    {
      std::ostringstream out;
      out << " Error couldn't claim the USB interface: " << usb_strerror(err) << std::endl
          << "Try to start xboxdrv with the option --detach-kernel-driver.";
      throw std::runtime_error(out.str());
    }
  }
}

Playstation3USBController::~Playstation3USBController()
{
  libusb_release_interface(m_handle, 0);
  libusb_close(m_handle);
}

void
Playstation3USBController::set_rumble(uint8_t left, uint8_t right)
{
  // not implemented
}

void
Playstation3USBController::set_led(uint8_t status)
{
  // not implemented
}

#define bitswap(x) x = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)

bool
Playstation3USBController::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  int len;
  uint8_t data[64] = {0};
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | endpoint_in, 
                                      data, sizeof(data), 
                                      &len, timeout);
  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  { // Error
    std::ostringstream str;
    str << "Playstation3USBController: USBError: " << ret << "\n" << usb_strerror(ret);
    throw std::runtime_error(str.str());
  }
  else
  {
    msg.type = XBOX_MSG_PS3USB;
    memcpy(&msg.ps3usb, data, sizeof(msg.ps3usb));

    bitswap(msg.ps3usb.accl_x);
    bitswap(msg.ps3usb.accl_y);
    bitswap(msg.ps3usb.accl_z);

    if (false)
    {
      if (false)
      {
        std::cout << boost::format("X:%5d Y:%5d Z:%5d\n") 
          % (static_cast<int>(msg.ps3usb.accl_x) - 512) 
          % (static_cast<int>(msg.ps3usb.accl_y) - 512)
          % (static_cast<int>(msg.ps3usb.accl_z) - 512);
      }
      else if (false)
      {
        std::cout << boost::format("X:%6.3f Y:%6.3f Z:%6.3f\n") 
          % ((static_cast<int>(msg.ps3usb.accl_x) - 512) / 116.0f)
          % ((static_cast<int>(msg.ps3usb.accl_y) - 512) / 116.0f)
          % ((static_cast<int>(msg.ps3usb.accl_z) - 512) / 116.0f);
      }
      // -116 is gravity
    }
    else if (false)
    {
      std::cout << len << ": ";
      for(int i = 0; i < len; ++i)
      {
        //std::cout << boost::format("%d:%02x ") % i % static_cast<int>(data[i]);
        std::cout << boost::format("%02x ") % static_cast<int>(data[i]);
      }
      std::cout << std::endl;
    }

    return true;   
  }
}

  /* EOF */
