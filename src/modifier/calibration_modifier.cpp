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

#include "calibration_modifier.hpp"

namespace {

int clamp(int lhs, int rhs, int v)
{
  return std::max(lhs, std::min(v, rhs));
}

} // namespace

CalibrationModifier::CalibrationModifier(const std::vector<CalibrationMapping>& calibration_map) :
  m_calibration_map(calibration_map)
{
}

void
CalibrationModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  for(std::vector<CalibrationMapping>::const_iterator i = m_calibration_map.begin(); i != m_calibration_map.end(); ++i)
  {
    int value = get_axis(msg,  i->axis);

    if (value < i->center)
      value = 32768 * (value - i->center) / (i->center - i->min);
    else if (value > i->center)
      value = 32767 * (value - i->center) / (i->max - i->center);
    else
      value = 0;

    set_axis(msg, i->axis, clamp(-32768, 32767, value));
  }
}

/* EOF */
