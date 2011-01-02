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

#include "autofire_modifier.hpp"

AutoFireModifier::AutoFireModifier(const std::vector<AutoFireMapping>& autofire_map) :
  m_autofire_map(autofire_map),
  m_button_timer()
{
  for(std::vector<AutoFireMapping>::const_iterator i = m_autofire_map.begin(); i != m_autofire_map.end(); ++i)
  {
    m_button_timer.push_back(0);
  }
}

void
AutoFireModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  for(size_t i = 0; i < m_autofire_map.size(); ++i)
  {
    if (get_button(msg, m_autofire_map[i].button))
    {
      m_button_timer[i] += msec_delta;

      if (m_button_timer[i] > m_autofire_map[i].frequency)
      {
        set_button(msg, m_autofire_map[i].button, 1);
        m_button_timer[i] = 0; // FIXME: we ignoring the passed time
      }
      else if (m_button_timer[i] > m_autofire_map[i].frequency/2)
      {
        set_button(msg, m_autofire_map[i].button, 0);
      }
      else
      {
        set_button(msg, m_autofire_map[i].button, 1);
      }
    }
    else
    {
      m_button_timer[i] = 0;
    }
  }
}

/* EOF */
