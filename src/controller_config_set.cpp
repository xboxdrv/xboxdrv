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

#include "controller_config_set.hpp"

ControllerConfigSet::ControllerConfigSet() :
  m_config(),
  m_current_config(0)
{
}

void
ControllerConfigSet::next_config()
{
  m_current_config += 1;

  if (m_current_config >= static_cast<int>(m_config.size()))
  {
    m_current_config = 0;
  }
}

void
ControllerConfigSet::prev_config()
{
  m_current_config -= 1;
  
  if (m_current_config < 0)
  {
    m_current_config = static_cast<int>(m_config.size()) - 1;
  }
}

int
ControllerConfigSet::config_count() const
{
  return static_cast<int>(m_config.size());
}

ControllerConfigPtr
ControllerConfigSet::get_config(int i) const
{
  assert(i >= 0);
  assert(i < static_cast<int>(m_config.size()));

  return m_config[i];
}

ControllerConfigPtr
ControllerConfigSet::get_config() const
{
  assert(!m_config.empty());

  return m_config[m_current_config];
}

void
ControllerConfigSet::add_config(ControllerConfigPtr config)
{
  m_config.push_back(config);
}

/* EOF */
