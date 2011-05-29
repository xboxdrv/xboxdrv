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

#include "modifier/button2axis_modifier.hpp"

#include <stdexcept>

#include "raise_exception.hpp"

Button2AxisModifier*
Button2AxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "BUTTON:BUTTON:AXIS required as argument");
  }
  else
  {
    XboxButton lhs_btn = string2btn(args[0]);
    XboxButton rhs_btn = string2btn(args[1]);
    XboxAxis   axis    = string2axis(args[2]);

    return new Button2AxisModifier(lhs_btn, rhs_btn, axis);
  }
}

Button2AxisModifier::Button2AxisModifier(XboxButton lhs_btn,
                                         XboxButton rhs_btn,
                                         XboxAxis   axis) :
  m_lhs_btn(lhs_btn),
  m_rhs_btn(rhs_btn),
  m_axis(axis)
{  
}

void
Button2AxisModifier::update(int msec_delta, XboxGenericMsg& msg) 
{
  bool lhs = msg.get_button(m_lhs_btn);
  bool rhs = msg.get_button(m_rhs_btn);

  if (lhs && !rhs)
  {
    msg.set_axis(m_axis, msg.get_axis_min(m_axis));
  }
  else if (!lhs && rhs)
  {
    msg.set_axis(m_axis, msg.get_axis_max(m_axis));
  }
  else
  {
    msg.set_axis(m_axis, 0);
  }
}

std::string
Button2AxisModifier::str() const
{
  return "btn2axis";
}
  
/* EOF */
