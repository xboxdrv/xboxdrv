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

#include "uinput/ui_key_event_collector.hpp"

#include <assert.h>

#include "log.hpp"
#include "uinput/uinput.hpp"

UIKeyEventCollector::UIKeyEventCollector(UInput& uinput, uint32_t device_id, int type, int code) :
  UIEventCollector(uinput, device_id, type, code),
  m_emitters(),
  m_value(0)
{
}

UIEventEmitterPtr
UIKeyEventCollector::create_emitter()
{
  UIKeyEventEmitterPtr emitter(new UIKeyEventEmitter(*this));
  m_emitters.push_back(emitter);
  return m_emitters.back();
}

void
UIKeyEventCollector::send(int value)
{
  assert(value == 0 || value == 1);

  if (value)
  {
    if (m_value >= static_cast<int>(m_emitters.size()))
    {
      log_error("got press event while all emitter where already pressed");
    }

    m_value += 1;

    if (m_value == 1)
    {
      m_uinput.send(get_device_id(), get_type(), get_code(), m_value);
    }
  }
  else
  {
    if (m_value <= 0)
    {
      log_error("got release event while collector was in release state");
    }

    m_value -= 1;

    if (m_value == 0)
    {
      m_uinput.send(get_device_id(), get_type(), get_code(), 0);
    }
  }
}

void
UIKeyEventCollector::sync()
{
}

/* EOF */
