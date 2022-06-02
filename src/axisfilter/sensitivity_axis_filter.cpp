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

#include "sensitivity_axis_filter.hpp"

#include <math.h>
#include <sstream>

#include <strut/split.hpp>

#include "util/math.hpp"
#include "util/string.hpp"

namespace xboxdrv {

SensitivityAxisFilter*
SensitivityAxisFilter::from_string(const std::string& str)
{
  auto tokens = strut::split(str, ':');

  float sensitivity = 0.0f;

  int j = 0;
  for(auto i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0: sensitivity = str2float(*i); break;
      default: throw std::runtime_error("to many arguments");
    };
  }

  return new SensitivityAxisFilter(sensitivity);
}

SensitivityAxisFilter::SensitivityAxisFilter(float sensitivity) :
  m_sensitivity(sensitivity)
{
}

int
SensitivityAxisFilter::filter(int value, int min, int max)
{
  float pos = to_float(value, min, max);

  float t = powf(2, m_sensitivity);

  // FIXME: there might be better/more standard ways to accomplish this
  if (pos > 0)
  {
    pos = powf(1.0f - powf(1.0f - pos, t), 1 / t);
    return from_float(pos, min, max);
  }
  else
  {
    pos = powf(1.0f - powf(1.0f - -pos, t), 1 / t);
    return from_float(-pos, min, max);
  }
}

std::string
SensitivityAxisFilter::str() const
{
  std::ostringstream out;
  out << "sensitivity:" << m_sensitivity;
  return out.str();
}

} // namespace xboxdrv

/* EOF */
