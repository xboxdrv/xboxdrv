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

#include <math.h>

#include "axis_sensitivty_modifier.hpp"

AxisSensitivityModifier::AxisSensitivityModifier(const std::vector<AxisSensitivityMapping>& axis_sensitivity_map) :
  m_axis_sensitivity_map(axis_sensitivity_map)
{
}

void
AxisSensitivityModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  for(std::vector<AxisSensitivityMapping>::const_iterator i = m_axis_sensitivity_map.begin();
      i != m_axis_sensitivity_map.end(); ++i)
  {
    float pos = get_axis_float(msg, i->axis);
    float t = powf(2, i->sensitivity);

    if (pos > 0)
    {
      pos = powf(1.0f - powf(1.0f - pos, t), 1 / t);
      set_axis_float(msg, i->axis, pos);
    }
    else
    {
      pos = powf(1.0f - powf(1.0f - -pos, t), 1 / t);
      set_axis_float(msg, i->axis, -pos);
    }
  }
}

/* EOF */
