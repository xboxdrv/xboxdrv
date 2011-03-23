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

#ifndef HEADER_FIRESTORM_DUAL_CONTROLLER_HPP
#define HEADER_FIRESTORM_DUAL_CONTROLLER_HPP

#include <libusb.h>
#include "xboxmsg.hpp"
#include "usb_controller.hpp"

class FirestormDualController : public USBController
{
private:
  bool is_vsb;
  int left_rumble;
  int right_rumble;

public:
  FirestormDualController(libusb_device* dev, bool is_vsb, bool try_detach);
  ~FirestormDualController();

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);

  bool parse(uint8_t* data, int len, XboxGenericMsg* msg_out);

private:
  bool parse_default(uint8_t* data, int len, XboxGenericMsg* msg_out);
  bool parse_vsb(uint8_t* data, int len, XboxGenericMsg* msg_out);

private:
  FirestormDualController(const FirestormDualController&);
  FirestormDualController& operator=(const FirestormDualController&);
};

#endif

/* EOF */
