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

#include "modifier/split_axis_modifier.hpp"

#include <sstream>
#include <stdexcept>

#include "raise_exception.hpp"

namespace xboxdrv {

SplitAxisModifier*
SplitAxisModifier::from_string(std::vector<std::string> const& args)
{
  if (args.size() != 3)
  {
    raise_exception(std::runtime_error, "SplitAxisModifier requires three arguments");
  }
  else
  {
    return new SplitAxisModifier(args[0], args[1], args[2]);
  }
}

SplitAxisModifier::SplitAxisModifier(std::string const& axis, std::string const& out_lhs, std::string const& out_rhs) :
  m_axis(axis),
  m_out_lhs(out_lhs),
  m_out_rhs(out_rhs)
{
}

void
SplitAxisModifier::init(ControllerMessageDescriptor& desc)
{
  m_axis.init(desc);
  m_out_lhs.init(desc);
  m_out_rhs.init(desc);
}

void
SplitAxisModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  float value = m_axis.get_float(msg);
  if (value < 0)
  {
    m_out_lhs.set_float(msg, -value * 2.0f - 1.0f);
    m_out_rhs.set_float(msg, -1.0f);
  }
  else if (value > 0)
  {
    m_out_lhs.set_float(msg, -1.0f);
    m_out_rhs.set_float(msg, value * 2.0f - 1.0f);
  }
  else
  {
    m_out_lhs.set_float(msg, -1.0f);
    m_out_rhs.set_float(msg, -1.0f);
  }
}

std::string
SplitAxisModifier::str() const
{
  std::ostringstream os;
  os << "split-axis:" << m_axis.str() << ":" << m_out_lhs.str() << ":" << m_out_rhs.str() << std::endl;
  return os.str();
}

} // namespace xboxdrv

/* EOF */
