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

#ifndef HEADER_XBOX_CONTROLLER_HPP
#define HEADER_XBOX_CONTROLLER_HPP

#include <usb.h>

struct XPadDevice;

/** */
class XboxController
{
private:
  struct usb_device* dev;
  XPadDevice*        dev_type;
  struct usb_dev_handle* handle;
  
public:
  XboxController(struct usb_device* dev,
                    XPadDevice*        dev_type);

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);
  void read(XboxGenericMsg& msg);

private:
  XboxController (const XboxController&);
  XboxController& operator= (const XboxController&);
};

#endif

/* EOF */
