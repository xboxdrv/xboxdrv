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

#ifndef HEADER_XBOX360_CONTROLLER_HPP
#define HEADER_XBOX360_CONTROLLER_HPP

#include <libusb.h>
#include <memory>
#include <string>

#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

class Chatpad;
class Headset;
struct XPadDevice;

class Xbox360Controller : public USBController
{
private:
  XPadDevice*        dev_type;

  int endpoint_in;
  int endpoint_out;

  std::unique_ptr<Chatpad> m_chatpad;
  std::unique_ptr<Headset> m_headset;

  uint8_t m_rumble_left;
  uint8_t m_rumble_right;

  Xbox360DefaultNames xbox;

public:
  Xbox360Controller(libusb_device* dev,
                    bool chatpad, bool chatpad_no_init, bool chatpad_debug,
                    bool headset,
                    bool headset_debug,
                    const std::string& headset_dump,
                    const std::string& headset_play,
                    bool try_detach);
  ~Xbox360Controller();

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;
  bool parse(const uint8_t* data, int len, ControllerMessage* msg_out) override;

private:
  Xbox360Controller (const Xbox360Controller&);
  Xbox360Controller& operator= (const Xbox360Controller&);
};

#endif

/* EOF */
