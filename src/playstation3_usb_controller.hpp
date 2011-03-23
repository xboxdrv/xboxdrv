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

#ifndef HEADER_XBOXDRV_PLAYSTATION3_USB_CONTROLLER_HPP
#define HEADER_XBOXDRV_PLAYSTATION3_USB_CONTROLLER_HPP

#include <libusb.h>

#include "usb_controller.hpp"

class Playstation3USBController : public USBController
{
private:
  int endpoint_in;
  int endpoint_out;

public:
  Playstation3USBController(libusb_device* dev, bool try_detach);
  ~Playstation3USBController();

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);

  bool parse(uint8_t* data, int len, XboxGenericMsg* msg_out);

private:
  Playstation3USBController(const Playstation3USBController&);
  Playstation3USBController& operator=(const Playstation3USBController&);
};

#endif

/* EOF */
