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

#include "controller_config.hpp"
#include "raise_exception.hpp"

Button2AxisModifier*
Button2AxisModifier::from_string(const std::vector<std::string>& args,
                                 const ControllerMessageDescriptor& msg_desc)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "BUTTON:BUTTON:AXIS required as argument");
  }
  else
  {
    int lhs_btn = msg_desc.get_key(args[0]);
    int rhs_btn = msg_desc.get_key(args[1]);
    int   axis    = msg_desc.get_abs(args[2]);

    return new Button2AxisModifier(lhs_btn, rhs_btn, axis);
  }
}

Button2AxisModifier::Button2AxisModifier(int lhs_btn,
                                         int rhs_btn,
                                         int axis) :
  m_lhs_btn(lhs_btn),
  m_rhs_btn(rhs_btn),
  m_axis(axis)
{  
}

void
Button2AxisModifier::update(int msec_delta, ControllerMessage& msg) 
{
  bool lhs = msg.get_key(m_lhs_btn);
  bool rhs = msg.get_key(m_rhs_btn);

  if (lhs && !rhs)
  {
    msg.set_abs(m_axis, msg.get_abs_min(m_axis));
  }
  else if (!lhs && rhs)
  {
    msg.set_abs(m_axis, msg.get_abs_max(m_axis));
  }
  else
  {
    msg.set_abs(m_axis, 0);
  }
}

std::string
Button2AxisModifier::str() const
{
  return "btn2axis";
}
  
/* EOF */
