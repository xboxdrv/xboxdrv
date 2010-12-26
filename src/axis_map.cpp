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

#include "axis_map.hpp"

AxisMap::AxisMap() :
  m_axis_map()
{
  clear();
}

void
AxisMap::bind(XboxAxis code, AxisEventPtr event)
{
  m_axis_map[XBOX_BTN_UNKNOWN][code] = event;
}

void
AxisMap::bind(XboxButton shift_code, XboxAxis code, AxisEventPtr event)
{
  m_axis_map[shift_code][code] = event;
}

AxisEventPtr
AxisMap::lookup(XboxAxis code) const
{
  return m_axis_map[XBOX_BTN_UNKNOWN][code];
}

AxisEventPtr
AxisMap::lookup(XboxButton shift_code, XboxAxis code) const
{
  return m_axis_map[shift_code][code];
}

void
AxisMap::clear()
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_AXIS_MAX; ++code)
    {
      m_axis_map[shift_code][code] = AxisEvent::invalid();
    }
  }
}

void
AxisMap::init(uInput& uinput) const
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_AXIS_MAX; ++code)
    {
      if (m_axis_map[shift_code][code])
      {
        m_axis_map[shift_code][code]->init(uinput);
      }
    }
  }
}

void
AxisMap::update(uInput& uinput, int msec_delta)
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_AXIS_MAX; ++code)
    {
      if (m_axis_map[shift_code][code])
      {
        m_axis_map[shift_code][code]->update(uinput, msec_delta);
      }
    }
  }
}

/* EOF */
