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

#ifndef HEADER_XBOXDRV_CONTROLLER_MESSAGE_HPP
#define HEADER_XBOXDRV_CONTROLLER_MESSAGE_HPP

#include <bitset>

#include "xboxmsg.hpp"

class ControllerMessage
{
private:
  int  m_axis_state[XBOX_AXIS_MAX];
  std::bitset<XBOX_BTN_MAX> m_button_state;

  std::bitset<XBOX_AXIS_MAX> m_axis_set;
  std::bitset<XBOX_BTN_MAX>  m_button_set;

public:
  ControllerMessage();
 
  void clear();

  bool get_key(int key) const;
  void set_key(int key, bool v);

  bool get_button(XboxButton button) const;
  void set_button(XboxButton button, bool v);

  int  get_axis(XboxAxis axis) const;
  void set_axis(XboxAxis axis, int v);

  float get_axis_float(XboxAxis axis) const;
  void  set_axis_float(XboxAxis axis, float v);

  static int get_axis_min(XboxAxis axis);
  static int get_axis_max(XboxAxis axis);

  void set_axis_min(XboxAxis axis, int value);
  void set_axis_max(XboxAxis axis, int value);

  bool axis_is_set(XboxAxis axis) const;
  bool button_is_set(XboxButton button) const;
};

std::ostream& operator<<(std::ostream& out, const ControllerMessage& msg);

#endif

/* EOF */
