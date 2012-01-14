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

#include "modifier/log_modifier.hpp"

#include <iostream>
#include <sstream>

LogModifier*
LogModifier::from_string(const std::string& value, const ControllerMessageDescriptor& msg_desc)
{
  return new LogModifier(value);
}

LogModifier::LogModifier(const std::string& prefix) :
  m_prefix(prefix)
{
}

void
LogModifier::update(int msec_delta, ControllerMessage& msg)
{
  std::cout << m_prefix << ": ";
  for(int i = 1; i < XBOX_BTN_MAX; ++i)
  {
    std::cout << msg.get_key(i);
    if (i != XBOX_BTN_MAX - 1)
      std::cout << " ";
  }

  std::cout << "  ";

  for(int i = 1; i < XBOX_AXIS_MAX; ++i)
  {
    std::cout << msg.get_abs(i);
    if (i != XBOX_AXIS_MAX - 1)
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

/* EOF */
