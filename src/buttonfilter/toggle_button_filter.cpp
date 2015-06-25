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

#include "buttonfilter/toggle_button_filter.hpp"

ToggleButtonFilter::ToggleButtonFilter() :
  m_state(false),
  m_last_value(false)
{
}

bool
ToggleButtonFilter::filter(bool value)
{
  if (value != m_last_value)
  {
    if (value)
    {
      m_state = !m_state;
    }

    m_last_value = value;
  }
  return m_state;
}

std::string
ToggleButtonFilter::str() const
{
  return "toggle";
}

/* EOF */
