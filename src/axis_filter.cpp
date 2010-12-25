/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "axis_filter.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

AxisFilterPtr
AxisFilter::from_string(const std::string& str)
{
  std::string::size_type p = str.find(':');
  const std::string& filtername = str.substr(0, p);

  if (filtername == "invert")
  {
    return AxisFilterPtr(new InvertAxisFilter);
  }
  else
  {
    std::ostringstream out;
    out << "unknown AxisFilter '" << filtername << "'";
    throw std::runtime_error(out.str());
  }
}

int
InvertAxisFilter::filter(int old_value, int value)
{
  return -value;
}

/* EOF */
