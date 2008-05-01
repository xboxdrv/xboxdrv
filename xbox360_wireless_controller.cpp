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

#include <usb.h>
#include <assert.h>
#include <sstream>
#include <boost/format.hpp>
#include <stdexcept>
#include "xboxmsg.hpp"
#include "xbox360_wireless_controller.hpp"

Xbox360WirelessController::Xbox360WirelessController(struct usb_device* dev,
                                                     XPadDevice*        dev_type,
                                                     int controller_id)
{
  assert(controller_id >= 0 && controller_id < 4);
  
  // FIXME: Is hardcoding those ok?
  endpoint  = controller_id*2 + 1;
  interface = controller_id*2;

  handle = usb_open(dev);
  if (!handle)
    {
      throw std::runtime_error("Xbox360WirelessController: Error opening Xbox360 controller");
    }
  else
    {
      if (usb_claim_interface(handle, interface) != 0) // FIXME: bInterfaceNumber shouldn't be hardcoded
        {
          std::ostringstream str;
          str << "Xbox360WirelessController: Error couldn't claim the USB interface " << interface;
          throw std::runtime_error(str.str());
        }
    }
}

Xbox360WirelessController::~Xbox360WirelessController()
{
  usb_release_interface(handle, interface); 
  usb_close(handle);
}

void
Xbox360WirelessController::set_rumble(uint8_t left, uint8_t right)
{
  //                                       +-- typo? might be 0x0c, i.e. length
  //                                       v
  char rumblecmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, left, right, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, endpoint, rumblecmd, sizeof(rumblecmd), 0);
}

void
Xbox360WirelessController::set_led(uint8_t status)
{
  //                                +--- Why not just status?
  //                                v
  char ledcmd[] = { 0x00, 0x00, 0x08, 0x40 + (status % 0x0e), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, endpoint, ledcmd, sizeof(ledcmd), 0);
}

void
Xbox360WirelessController::read(XboxGenericMsg& msg)
{
  uint8_t data[32];
  int ret = usb_interrupt_read(handle, endpoint, (char*)data, sizeof(data), 0 /*Timeout*/);
  
  if (ret < 0)
    { // Error
      std::ostringstream str;
      str << "USBError: " << ret << "\n" << usb_strerror();
      throw std::runtime_error(str.str());
    }
  else if (ret == 2 && data[0] == 0x08) 
    { // Connection Status Message
      if (data[1] == 0x00) 
        {
          // nothing connected
        } 
      else if (data[1] == 0x80) 
        {
          // controller connected
        } 
      else if (data[1] == 0x40) 
        {
          // headset connected
        }
      else if (data[1] == 0xc0) 
        {
          // headset and controller connected
        }
      else
        {
          // unknown
        }
    }
  else if (ret == 29) // Event Message
    {
      if (data[0] == 0x00 && data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
        { // Initial Announc Message
          std::string serial = (boost::format("%x:%x:%x:%x:%x:%x") 
                                % data[7] % data[8] % data[9] % data[10] % data[11] % data[12] % data[13]).str();
            
          battery_status = data[17];
        }
      else if (data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0xf0 && data[4] == 0x00 && data[5] == 0x13)
        {
          msg.type    = GAMEPAD_XBOX360_WIRELESS;
          msg.xbox360 = *reinterpret_cast<Xbox360Msg*>(&data[6]);
        }
      else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
        { // Battery status
          battery_status = data[4];
        }
      else
        {
          // unknown/junk
        }
    }
  else
    {
      // unknown/junk
    }
}

/* EOF */
