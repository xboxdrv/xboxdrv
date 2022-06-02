/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

namespace xboxdrv {

JoinAxisModifier*
JoinAxisModifier::from_string(std::vector<std::string> const& args)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "JoinAxisModifier requires three arguments");
  }
  else
  {
    return new JoinAxisModifier(args[0], args[1], args[2]);
  }
}

JoinAxisModifier::JoinAxisModifier(std::string const& lhs, std::string const& rhs, std::string const& out) :
  m_lhs(lhs),
  m_rhs(rhs),
  m_out(out)
{
}

void
JoinAxisModifier::init(ControllerMessageDescriptor& desc)
{
  m_lhs.init(desc);
  m_rhs.init(desc);
  m_out.init(desc);
}

void
JoinAxisModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  m_out.set_float(msg, (m_rhs.get_float(msg) - m_lhs.get_float(msg)) / 2.0f);
}

std::string
JoinAxisModifier::str() const
{
  std::ostringstream os;
  os << "join-axis:"
     << m_lhs.get_name() << ":"
     << m_rhs.get_name() << ":"
     << m_out.get_name();
  return os.str();
}

} // namespace xboxdrv

/* EOF */
