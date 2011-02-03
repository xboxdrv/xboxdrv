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

#include "controller_slot.hpp"

#include <boost/format.hpp>

ControllerSlot::ControllerSlot() :
  m_id(),
  m_config(),
  m_rules(),
  m_led_status(-1),
  m_thread()  
{}

ControllerSlot::ControllerSlot(int id_,
                               ControllerSlotConfigPtr config_,
                               std::vector<ControllerMatchRulePtr> rules_,
                               int led_status_,
                               XboxdrvThreadPtr thread_) :
  m_id(id_),
  m_config(config_),
  m_rules(rules_),
  m_led_status(led_status_),
  m_thread(thread_)
{}

void
ControllerSlot::connect(XboxdrvThreadPtr thread)
{
  assert(!m_thread);
  m_thread = thread;
}

XboxdrvThreadPtr
ControllerSlot::disconnect()
{
  XboxdrvThreadPtr thread = m_thread;
  m_thread.reset();
  return thread;
}

bool
ControllerSlot::try_disconnect()
{
  assert(m_thread);

  if (m_thread->try_join_thread())
  {
    disconnect();
    return true;
  }
  else
  {
    return false;
  }
}

bool
ControllerSlot::is_connected() const
{
  return m_thread;
}

/* EOF */
