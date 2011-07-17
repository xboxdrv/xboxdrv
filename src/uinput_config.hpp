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

#include <bitset>

#include "axis_map.hpp"
#include "button_map.hpp"

struct Xbox360Msg;
struct ControllerMessage;
struct Playstation3USBMsg;
struct XboxMsg;

class UInputOptions;

class UInputConfig
{
private:
  UInput& m_uinput;

  ButtonMap m_btn_map;
  AxisMap   m_axis_map;

  int  axis_state[XBOX_AXIS_MAX];
  std::bitset<XBOX_BTN_MAX> m_button_state;
  std::bitset<XBOX_BTN_MAX> m_last_button_state;

public:
  UInputConfig(UInput& uinput, int slot, bool extra_devices, const UInputOptions& opts);

  void send(const ControllerMessage& msg); 
  void update(int msec_delta);

  void reset_all_outputs();

private:
  void send_axis(XboxAxis code, int32_t value);

private:
  UInputConfig(const UInputConfig&);
  UInputConfig& operator=(const UInputConfig&);
};

#endif

/* EOF */
