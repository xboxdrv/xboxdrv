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

#include "axisfilter/log_axis_filter.hpp"

#include <iostream>
#include <sstream>

LogAxisFilter*
LogAxisFilter::from_string(const std::string& str)
{
  return new LogAxisFilter(str);
}

LogAxisFilter::LogAxisFilter(const std::string& name) :
  m_name(name)
{
}

int
LogAxisFilter::filter(int value, int min, int max)
{
  if (m_name.empty())
  {
    std::cout << value << std::endl;
  }
  else
  {
    std::cout << m_name << ": " << value << std::endl;    
  }

  return value;
}

std::string
LogAxisFilter::str() const
{
  std::ostringstream out;
  out << "log:" << m_name;
  return out.str();
}

/* EOF */
