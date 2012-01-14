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

#include "modifier/join_axis_modifier.hpp"

#include <sstream>
#include <stdexcept>

#include "raise_exception.hpp"

JoinAxisModifier*
JoinAxisModifier::from_string(const std::vector<std::string>& args,
                              const ControllerMessageDescriptor& msg_desc)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "JoinAxisModifier requires three arguments");
  }
  else
  {
    return new JoinAxisModifier(msg_desc.get_abs(args[0]),
                                msg_desc.get_abs(args[1]),
                                msg_desc.get_abs(args[2]));
  }
}

JoinAxisModifier::JoinAxisModifier(int lhs, int rhs, int out) :
  m_lhs(lhs),
  m_rhs(rhs),
  m_out(out)
{
}

void
JoinAxisModifier::update(int msec_delta, ControllerMessage& msg)
{
  msg.set_abs_float(m_out,
                     (msg.get_abs_float(m_rhs) - msg.get_abs_float(m_lhs)) / 2.0f);
}

std::string
JoinAxisModifier::str() const
{
  std::ostringstream os;
  /* BROKEN:
  os << "join-axis:" 
     << axis2string(m_lhs) << ":"
     << axis2string(m_rhs) << ":"
     << axis2string(m_out);
  */
  return os.str();
}

/* EOF */
