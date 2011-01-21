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

#ifndef HEADER_XBOXDRV_UINPUT_CONFIG_HPP
#define HEADER_XBOXDRV_UINPUT_CONFIG_HPP

#include <map>

#include "axis_map.hpp"
#include "button_map.hpp"

struct Xbox360Msg;
struct XboxGenericMsg;
struct XboxMsg;

class UInputOptions;

class UInputConfig
{
private:
  uInput& m_uinput;

  ButtonMap m_btn_map;
  AxisMap   m_axis_map;

  int  axis_state[XBOX_AXIS_MAX];
  bool button_state[XBOX_BTN_MAX];
  bool last_button_state[XBOX_BTN_MAX];

public:
  UInputConfig(uInput& uinput, const UInputOptions& opts);

  void send(XboxGenericMsg& msg); 
  void update(int msec_delta);

  void reset_all_outputs();

private:
  void send(Xbox360Msg& msg);
  void send(XboxMsg& msg);

  void send_button(XboxButton code, bool value);
  void send_axis(XboxAxis code, int32_t value);

private:
  UInputConfig(const UInputConfig&);
  UInputConfig& operator=(const UInputConfig&);
};

#endif

/* EOF */
