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
Button2AxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "BUTTON:BUTTON:AXIS required as argument");
  }
  else
  {
    return new Button2AxisModifier(args[0], args[1], args[2]);
  }
}

Button2AxisModifier::Button2AxisModifier(const std::string& lhs_btn,
                                         const std::string& rhs_btn,
                                         const std::string& axis) :
  m_lhs_btn_str(lhs_btn),
  m_rhs_btn_str(rhs_btn),
  m_axis_str(axis),
  m_lhs_btn(-1),
  m_rhs_btn(-1),
  m_axis(-1)
{  
}

void
Button2AxisModifier::init(ControllerMessageDescriptor& desc)
{
  m_lhs_btn = desc.key().get(m_lhs_btn_str);
  m_rhs_btn = desc.key().get(m_rhs_btn_str);
  m_axis = desc.abs().get(m_axis_str);
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
