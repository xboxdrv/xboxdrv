/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <stdexcept>
#include <sstream>
#include "xboxmsg.hpp"
#include "xbox360_controller.hpp"

Xbox360Controller::Xbox360Controller(struct usb_device* dev,
                                     XPadDevice*        dev_type)
{
  struct usb_dev_handle* handle = usb_open(dev);
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

void
Xbox360Controller::set_rumble(uint8_t left, uint8_t right)
{
  char rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, 2, rumblecmd, 8, 0);
}

void
Xbox360Controller::set_led(uint8_t status)
{
  char ledcmd[] = { 1, 3, status }; 
  usb_interrupt_write(handle, 2, ledcmd, sizeof(ledcmd), 0);
}

void
Xbox360Controller::read(Xbox360Msg& msg)
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
  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
    {
      msg = (Xbox360Msg&)data;
    }
}

/* EOF */
