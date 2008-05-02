/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>
#include "xboxmsg.hpp"
#include "xbox360_controller.hpp"

Xbox360Controller::Xbox360Controller(struct usb_device* dev, bool is_guitar)
  : is_guitar(is_guitar)
{
  handle = usb_open(dev);
  if (!handle)
    {
      throw std::runtime_error("Error opening Xbox360 controller");
    }
  else
    {
      if (usb_claim_interface(handle, 0) != 0) // FIXME: bInterfaceNumber shouldn't be hardcoded
        {
          throw std::runtime_error("Error couldn't claim the USB interface");
        }
    }
}

Xbox360Controller::~Xbox360Controller()
{
  usb_release_interface(handle, 0); 
  usb_close(handle);
}

void
Xbox360Controller::send_raw(char* buffer, int len)
{
  usb_interrupt_write(handle, 2, buffer, len, 0);  
}

void
Xbox360Controller::set_rumble(uint8_t left, uint8_t right)
{
  char rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, 2, rumblecmd, sizeof(rumblecmd), 0);
}

void
Xbox360Controller::set_led(uint8_t status)
{
  char ledcmd[] = { 0x01, 0x03, status }; 
  usb_interrupt_write(handle, 2, ledcmd, sizeof(ledcmd), 0);
}

bool
Xbox360Controller::read(XboxGenericMsg& msg)
{
  uint8_t data[32];
  int ret = usb_interrupt_read(handle, 1 /*EndPoint*/, (char*)data, sizeof(data), 0 /*Timeout*/);

  if (ret < 0)
    { // Error
      std::ostringstream str;
      str << "USBError: " << ret << "\n" << usb_strerror();
      throw std::runtime_error(str.str());
    }
  else if (ret == 0)
    {
      // happens with the Xbox360 controller every now and then, just
      // ignore, seems harmless, so just ignore
    }
  else if (ret == 3 && data[0] == 0x01 && data[1] == 0x03)
    { 
      // std::cout << "Xbox360Controller: LED Status: " << int(data[2]) << std::endl;
    }
  else if (ret == 3 && data[0] == 0x03 && data[1] == 0x03)
    { 
      // data[2] == 0x00 means that rumble is disabled
      // data[2] == 0x01 unknown, but rumble works
      // data[2] == 0x02 unknown, but rumble works
      // data[2] == 0x03 is default with rumble enabled
      // std::cout << "Xbox360Controller: Rumble Status: " << int(data[2]) << std::endl;
    }
  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
    {
      if (is_guitar)
        {
          msg.type   = GAMEPAD_XBOX360_GUITAR;
          msg.guitar = *reinterpret_cast<Xbox360GuitarMsg*>(data);
        }
      else
        {
          msg.type    = GAMEPAD_XBOX360;
          msg.xbox360 = *reinterpret_cast<Xbox360Msg*>(data);
        }
      return true;
    }
  else
    {
      std::cout << "Unknown data: bytes: " << ret 
                << " Data: ";
                      
      for(int j = 0; j < ret; ++j)
        std::cout << boost::format("0x%02x ") % int(data[j]);

      std::cout << std::endl;
    }

  return false;
}

/* EOF */
