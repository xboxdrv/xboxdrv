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

#include "axisfilter/const_axis_filter.hpp"

#include <sstream>

#include "util/string.hpp"

namespace xboxdrv {

ConstAxisFilter*
ConstAxisFilter::from_string(const std::string& rest)
{
  return new ConstAxisFilter(str2int(rest));
}

ConstAxisFilter::ConstAxisFilter(int value) :
  m_value(value)
{}

int
ConstAxisFilter::filter(int value, int min, int max)
{
  return m_value;
}

std::string
ConstAxisFilter::str() const
{
  std::ostringstream os;
  os << "const:" << m_value;
  return os.str();
}

} // namespace xboxdrv

/* EOF */
