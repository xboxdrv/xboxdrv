/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SAITEK_P3600_CONTROLLER_HPP
#define HEADER_SAITEK_P3600_CONTROLLER_HPP

#include <libusb.h>
#include "xboxmsg.hpp"
#include "usb_controller.hpp"

class SaitekP3600Controller : public USBController
{
private:
  int left_rumble;
  int right_rumble;

public:
  SaitekP3600Controller(libusb_device* dev, bool try_detach);
  ~SaitekP3600Controller();

  void set_rumble_real(uint8_t left, uint8_t right);
  void set_led_real(uint8_t status);

  bool parse(uint8_t* data, int len, XboxGenericMsg* msg_out);

private:
  SaitekP3600Controller(const SaitekP3600Controller&);
  SaitekP3600Controller& operator=(const SaitekP3600Controller&);
};

#endif

/* EOF */
