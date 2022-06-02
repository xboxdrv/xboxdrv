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

#include "modifier/log_modifier.hpp"

#include <iostream>
#include <sstream>

namespace xboxdrv {

LogModifier*
LogModifier::from_string(std::string const& value)
{
  return new LogModifier(value);
}

LogModifier::LogModifier(std::string const& prefix) :
  m_prefix(prefix)
{
}

void
LogModifier::init(ControllerMessageDescriptor& desc)
{
}

void
LogModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  std::cout << m_prefix << ": ";
  for(int i = 0; i < desc.get_key_count(); ++i)
  {
    std::cout << msg.get_key(i);
    if (i != desc.get_key_count() - 1)
      std::cout << " ";
  }

  std::cout << "  ";

  for(int i = 0; i < desc.get_abs_count(); ++i)
  {
    std::cout << msg.get_abs(i);
    if (i != desc.get_abs_count() - 1)
      std::cout << " ";
  }
  std::cout << std::endl;
}

std::string
LogModifier::str() const
{
  std::ostringstream os;
  os << "log:" << m_prefix;
  return os.str();
}

} // namespace xboxdrv

/* EOF */
