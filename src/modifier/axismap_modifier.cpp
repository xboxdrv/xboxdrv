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

#include "axismap_modifier.hpp"

/** converts the arbitary range to [-1,1] */
inline float to_float(int value, int min, int max)
{
  return static_cast<float>(value - min) / static_cast<float>(max - min) * 2.0f - 1.0f;
}

AxismapModifier::AxismapModifier(const std::vector<AxisMapping>& axismap) :
  m_axismap(axismap)
{
}

void
AxismapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  // update all filters in all mappings
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    for(std::vector<AxisFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      (*j)->update(msec_delta);
    }
  }

  // clear all values in the new msg
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    set_axis_float(newmsg, i->lhs, 0);
  }

  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    int min = get_axis_min(i->lhs);
    int max = get_axis_max(i->lhs);
    int value = get_axis(msg, i->lhs);

    for(std::vector<AxisFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      value = (*j)->filter(value, min, max);
    }

    float lhs  = to_float(value, min, max);
    float nrhs = get_axis_float(newmsg, i->rhs);

    if (i->invert)
    {
      lhs = -lhs;
    }

    set_axis_float(newmsg, i->rhs, std::max(std::min(nrhs + lhs, 1.0f), -1.0f));
  }
  msg = newmsg;
}

/* EOF */
