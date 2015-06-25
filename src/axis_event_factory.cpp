/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#include "axis_event_factory.hpp"

#include <linux/uinput.h>

#include "evdev_helper.hpp"

#include "axisevent/abs_axis_event_handler.hpp"
#include "axisevent/key_axis_event_handler.hpp"
#include "axisevent/log_axis_event_handler.hpp"
#include "axisevent/rel_axis_event_handler.hpp"
#include "axisevent/rel_repeat_axis_event_handler.hpp"
#include "axisevent/rumble_axis_event_handler.hpp"

AxisEventFactory::AxisEventFactory(UInput& uinput, int slot, bool extra_devices) :
  m_uinput(uinput),
  m_slot(slot),
  m_extra_devices(extra_devices)
{
}

AxisEventPtr
AxisEventFactory::invalid()
{
  return AxisEventPtr();
}

AxisEventPtr
AxisEventFactory::from_string(const std::string& str)
{
  AxisEventPtr ev;

  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos)
    rest = str.substr(p+1);

  if (token == "abs")
  {
    ev.reset(new AxisEvent(AbsAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices, rest)));
  }
  else if (token == "rel")
  {
    ev.reset(new AxisEvent(RelAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices, rest)));
  }
  else if (token == "rel-repeat")
  {
    ev.reset(new AxisEvent(RelRepeatAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices, rest)));
  }
  else if (token == "key")
  {
    ev.reset(new AxisEvent(KeyAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices, rest)));
  }
  else if (token == "log")
  {
    ev.reset(new AxisEvent(new LogAxisEventHandler(rest)));
  }
  else if (token == "rumble")
  {
    ev.reset(new AxisEvent(RumbleAxisEventHandler::from_string(rest)));
  }
  else
  { // try to guess a type
    switch (get_event_type(str))
    {
      case EV_ABS:
        ev.reset(new AxisEvent(AbsAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices,
                                                                str)));
        break;

      case EV_REL:
        ev.reset(new AxisEvent(RelAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices,
                                                                str)));
        break;

      case EV_KEY:
        ev.reset(new AxisEvent(KeyAxisEventHandler::from_string(m_uinput, m_slot, m_extra_devices,
                                                                str)));
        break;

      case -1: // void/none
        ev = invalid();
        break;

      default:
        assert(!"should never be reached");
    }
  }

  return ev;
}

/* EOF */
