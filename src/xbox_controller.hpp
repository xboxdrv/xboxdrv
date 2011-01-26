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

#include <libusb.h>

#include "xbox_generic_controller.hpp"

struct XPadDevice;

class XboxController : public XboxGenericController
{
private:
  libusb_device* dev;
  libusb_device_handle* handle;

  int endpoint_in;
  int endpoint_out;
  
  void find_endpoints();

public:
  XboxController(libusb_device* dev, bool try_detach);
  virtual ~XboxController();

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);
  bool read(XboxGenericMsg& msg, int timeout);

private:
  XboxController (const XboxController&);
  XboxController& operator= (const XboxController&);
};

#endif

/* EOF */
