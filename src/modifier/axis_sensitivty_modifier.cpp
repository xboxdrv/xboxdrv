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

#include "axis_sensitivty_modifier.hpp"

#include <math.h>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

AxisSensitivityModifier*
AxisSensitivityModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 2)
  {
    throw std::runtime_error("AxisSensitivityModifier takes exactly two arguments");
  }
  else
  {
    return new AxisSensitivityModifier(string2axis(args[0]),
                                       boost::lexical_cast<float>(args[1]));
  }
}

AxisSensitivityModifier*
AxisSensitivityModifier::from_string(const std::string& lhs, const std::string& rhs)
{
  /* 
     Format of str: X1=SENSITIVITY
     Example: X1=2.0
  */
  return new AxisSensitivityModifier(string2axis(lhs),
                                     boost::lexical_cast<float>(rhs));
}

AxisSensitivityModifier::AxisSensitivityModifier(XboxAxis axis, float sensitivity) :
  m_axis(axis),
  m_sensitivity(sensitivity)
{
}

void
AxisSensitivityModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  float pos = get_axis_float(msg, m_axis);
  float t = powf(2, m_sensitivity);

  if (pos > 0)
  {
    pos = powf(1.0f - powf(1.0f - pos, t), 1 / t);
    set_axis_float(msg, m_axis, pos);
  }
  else
  {
    pos = powf(1.0f - powf(1.0f - -pos, t), 1 / t);
    set_axis_float(msg, m_axis, -pos);
  }
}

/* EOF */
