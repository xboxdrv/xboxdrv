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

#include "ui_event_collector.hpp"

#include <assert.h>

#include "log.hpp"
#include "uinput.hpp"

UIEventCollector::UIEventCollector(UInput& uinput, 
                               uint32_t device_id, 
                               int type, 
                               int code) :
  m_uinput(uinput),
  m_device_id(device_id),
  m_type(type),
  m_code(code),
  m_value(0),
  m_emitters()
{
  assert(m_code != -1);
}

UIEventEmitterPtr
UIEventCollector::create_emitter()
{
  UIEventEmitterPtr emitter(new UIEventEmitter(*this));
  m_emitters.push_back(emitter);
  return emitter;
}

void
UIEventCollector::sync()
{
  int value = 0;
  for(Emitters::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
  {
    switch(m_type)
    {
      case EV_KEY:
        value = value || (*i)->get_value();
        break;

      case EV_REL:
        value += (*i)->get_value();
        break;

      case EV_ABS:
        value = (*i)->get_value();
        break;

      default:
        assert(!"unknown type");
    }
  }

  if (value != m_value)
  {
    m_value = value;
    m_uinput.send(m_device_id, m_type, m_code, m_value);
  }
}

/* EOF */
