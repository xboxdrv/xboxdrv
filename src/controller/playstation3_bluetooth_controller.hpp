/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_CONTROLLER_PLAYSTATION3_BLUETOOTH_CONTROLLER_HPP
#define HEADER_XBOXDRV_CONTROLLER_PLAYSTATION3_BLUETOOTH_CONTROLLER_HPP

#include <libusb.h>

#include "controller.hpp"

namespace xboxdrv {

class Playstation3BluetoothController : public Controller
{
private:
public:
  Playstation3BluetoothController();
  ~Playstation3BluetoothController();

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;

private:
  Playstation3BluetoothController(const Playstation3BluetoothController&);
  Playstation3BluetoothController& operator=(const Playstation3BluetoothController&);
};

} // namespace xboxdrv

#endif

/* EOF */
