// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "event.hpp"

#include <cassert>
#include <fmt/format.h>

#include "linux.hpp"
#include "multi_device.hpp"
#include "parse.hpp"

namespace uinpp {

Event
Event::create(uint16_t device_id, int type, int code)
{
  Event ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = device_id;
  ev.m_device_id_resolved = false;
  ev.type      = type;
  ev.code      = code;
  return ev;
}

Event
Event::invalid()
{
  Event ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_INVALID;
  ev.m_device_id_resolved = false;
  ev.type      = -1;
  ev.code      = -1;
  return ev;
}

bool
Event::operator<(Event const& rhs)  const
{
  // BROKEN: must take all members into account
  if (m_device_id == rhs.m_device_id)
  {
    if (type == rhs.type)
    {
      return code < rhs.code;
    }
    else if (type > rhs.type)
    {
      return false;
    }
    else // if (type < rhs.type)
    {
      return true;
    }
  }
  else if (m_device_id > rhs.m_device_id)
  {
    return false;
  }
  else // if (device_id < rhs.device_id)
  {
    return true;
  }
}

void
Event::resolve_device_id(int slot, bool extra_devices)
{
  assert(!m_device_id_resolved);

  if (m_slot_id == SLOTID_AUTO)
  {
    m_slot_id = static_cast<uint16_t>(slot);
  }

  if (m_device_id == DEVICEID_AUTO)
  {
    if (extra_devices)
    {
      switch(type)
      {
        case EV_KEY:
          if (is_mouse_button(code))
          {
            m_device_id = DEVICEID_MOUSE;
          }
          else if (is_keyboard_button(code))
          {
            m_device_id = DEVICEID_KEYBOARD;
          }
          else
          {
            m_device_id = DEVICEID_JOYSTICK;
          }
          break;

        case EV_REL:
          m_device_id = DEVICEID_MOUSE;
          break;

        case EV_ABS:
          m_device_id = DEVICEID_JOYSTICK;
          break;
      }
    }
    else
    {
      m_device_id = DEVICEID_JOYSTICK;
    }
  }

  m_device_id_resolved = true;
}

uint32_t
Event::get_device_id() const
{
  assert(m_device_id_resolved);

  return create_device_id(m_slot_id, m_device_id);
}

} // namespace uinpp

/* EOF */
