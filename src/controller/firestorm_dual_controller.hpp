/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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
#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

namespace xboxdrv {

class FirestormDualController : public USBController
{
private:
  bool is_vsb;

  Xbox360DefaultNames xbox;

public:
  FirestormDualController(libusb_device* dev, bool is_vsb, bool try_detach);
  ~FirestormDualController();

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;

  bool parse(const uint8_t* data, int len, ControllerMessage* msg_out) override;

private:
  bool parse_default(const uint8_t* data, int len, ControllerMessage* msg_out);
  bool parse_vsb(const uint8_t* data, int len, ControllerMessage* msg_out);

private:
  FirestormDualController(const FirestormDualController&);
  FirestormDualController& operator=(const FirestormDualController&);
};

} // namespace xboxdrv

#endif

/* EOF */
