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

#include "ui_abs_event_collector.hpp"

#include "uinput.hpp"

UIAbsEventCollector::UIAbsEventCollector(UInput& uinput, uint32_t device_id, int type, int code) : 
  UIEventCollector(uinput, device_id, type, code),
  m_emitters()
{
}

UIEventEmitterPtr
UIAbsEventCollector::create_emitter()
{
  UIAbsEventEmitterPtr emitter(new UIAbsEventEmitter);
  m_emitters.push_back(emitter);
  return m_emitters.back();
}

void
UIAbsEventCollector::sync()
{
  for(Emitters::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
  {
    m_uinput.send(get_device_id(), get_type(), get_code(), (*i)->get_value());    
  }
}

/* EOF */
