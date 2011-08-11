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

#include "modifier/split_axis_modifier.hpp"

#include <sstream>
#include <stdexcept>

#include "raise_exception.hpp"

SplitAxisModifier*
SplitAxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "SplitAxisModifier requires three arguments");
  }
  else
  {
    return new SplitAxisModifier(string2axis(args[0]),
                                 string2axis(args[1]),
                                 string2axis(args[2]));
  }
}

SplitAxisModifier::SplitAxisModifier(XboxAxis axis, XboxAxis out_lhs, XboxAxis out_rhs) :
  m_axis(axis),
  m_out_lhs(out_lhs),
  m_out_rhs(out_rhs)
{
}

void
SplitAxisModifier::update(int msec_delta, ControllerMessage& msg)
{
  float value = msg.get_axis_float(m_axis);
  if (value < 0)
  {
    msg.set_axis_float(m_out_lhs, -value * 2.0f - 1.0f);
    msg.set_axis_float(m_out_rhs, -1.0f);
  }
  else if (value > 0)
  {
    msg.set_axis_float(m_out_lhs, -1.0f);
    msg.set_axis_float(m_out_rhs, value * 2.0f - 1.0f);
  }
  else
  {
    msg.set_axis_float(m_out_lhs, -1.0f);
    msg.set_axis_float(m_out_rhs, -1.0f);
  }
}

std::string
SplitAxisModifier::str() const
{
  std::ostringstream os;
  os << "split-axis:" 
     << axis2string(m_axis) << ":"
     << axis2string(m_out_lhs) << ":"
     << axis2string(m_out_rhs) << std::endl;
  return os.str();
}

/* EOF */
