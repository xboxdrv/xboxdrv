/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_UINPUT_CFG_HPP
#define HEADER_XBOXDRV_UINPUT_CFG_HPP

#include "axis_event.hpp"
#include "button_event.hpp"
#include "button_map.hpp"
#include "xboxmsg.hpp"

class uInputCfg
{
public:
  std::string device_name;

  bool trigger_as_button;
  bool trigger_as_zaxis;

  bool dpad_as_button;
  bool dpad_only;

  bool force_feedback;
  bool extra_devices;

  ButtonMap btn_map;
  AxisEvent axis_map[XBOX_AXIS_MAX];

  uInputCfg();

  /** Sets a button/axis mapping that is equal to the xpad kernel driver */
  void mimic_xpad();
};

#endif

/* EOF */
