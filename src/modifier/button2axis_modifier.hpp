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

#ifndef HEADER_XBOXDRV_MODIFIER_BUTTON2AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_BUTTON2AXIS_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"

class Button2AxisModifier : public Modifier
{
public:
  static Button2AxisModifier* from_string(const std::vector<std::string>& args,
                                          const ControllerMessageDescriptor& msg_desc);

private:
  int m_lhs_btn;
  int m_rhs_btn;
  int   m_axis;

public:
  Button2AxisModifier(int lhs_btn, int rhs_btn, int   axis);

  void update(int msec_delta, ControllerMessage& msg);
  std::string str() const;

private:
  Button2AxisModifier(const Button2AxisModifier&);
  Button2AxisModifier& operator=(const Button2AxisModifier&);
};

#endif

/* EOF */
