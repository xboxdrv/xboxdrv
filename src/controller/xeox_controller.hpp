/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
**  Copyright (C) 2014 Jan Hambrecht <jaham@gmx.net>
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

#ifndef HEADER_XBOXDRV_XEOX_CONTROLLER_HPP
#define HEADER_XBOXDRV_XEOX_CONTROLLER_HPP

#include <libusb.h>

#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

namespace xboxdrv {

/** Speedlink Xeox USB (SL-6555-SBK-A), USB ID 1a34:0802 — ACRUX HID report. */
class XeoxController : public USBController
{
private:
  int m_endpoint_in;
  int m_endpoint_out;

  Xbox360DefaultNames xbox;

public:
  XeoxController(libusb_device* dev, bool try_detach);
  ~XeoxController() override;

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;

  bool parse(uint8_t const* data, int len, ControllerMessage* msg_out) override;

private:
  XeoxController(const XeoxController&);
  XeoxController& operator=(const XeoxController&);
};

} // namespace xboxdrv

#endif

/* EOF */
