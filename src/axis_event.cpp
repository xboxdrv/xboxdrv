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

#include "axis_event.hpp"

#include <boost/tokenizer.hpp>
#include <math.h>

#include "evdev_helper.hpp"
#include "log.hpp"
#include "helper.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

#include "axisevent/abs_axis_event_handler.hpp"
#include "axisevent/key_axis_event_handler.hpp"
#include "axisevent/rel_axis_event_handler.hpp"
#include "axisevent/rel_repeat_axis_event_handler.hpp"

AxisEventPtr
AxisEvent::invalid() 
{ 
  return AxisEventPtr();
}

AxisEventPtr
AxisEvent::create_abs(int device_id, int code, int min, int max, int fuzz, int flat)
{
  return AxisEventPtr(new AxisEvent(new AbsAxisEventHandler(UIEvent::create(static_cast<uint16_t>(device_id),
                                                                            EV_ABS, code),
                                                            min, max, fuzz, flat),
                                    min, max));
}

AxisEventPtr
AxisEvent::create_rel(int device_id, int code, int repeat, float value)
{
  return AxisEventPtr(new AxisEvent(new RelAxisEventHandler(device_id, code, repeat, value)));
}
  
AxisEventPtr
AxisEvent::from_string(const std::string& str)
{
  AxisEventPtr ev;

  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (token == "abs")
  {
    ev.reset(new AxisEvent(AbsAxisEventHandler::from_string(rest)));
  }
  else if (token == "rel")
  {
    ev.reset(new AxisEvent(RelAxisEventHandler::from_string(rest)));
  }
  else if (token == "rel-repeat")
  {
    ev.reset(new AxisEvent(RelRepeatAxisEventHandler::from_string(rest)));
  }
  else if (token == "key")
  {
    ev.reset(new AxisEvent(KeyAxisEventHandler::from_string(rest)));
  }
  else
  { // try to guess a type
    switch (get_event_type(str))
    {
      case EV_ABS:
        ev.reset(new AxisEvent(AbsAxisEventHandler::from_string(str)));
        break;

      case EV_REL:
        ev.reset(new AxisEvent(RelAxisEventHandler::from_string(str)));
        break;

      case EV_KEY:
        ev.reset(new AxisEvent(KeyAxisEventHandler::from_string(str)));
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

AxisEvent::AxisEvent(AxisEventHandler* handler, int min, int max) :
  m_last_raw_value(0),
  m_last_send_value(0),
  m_min(min),
  m_max(max),
  m_handler(handler),
  m_filters()
{
}

void
AxisEvent::add_filter(AxisFilterPtr filter)
{
  m_filters.push_back(filter);
}

void
AxisEvent::init(UInput& uinput, int slot, bool extra_devices)
{
  m_handler->init(uinput, slot, extra_devices);
}

void
AxisEvent::send(UInput& uinput, int value)
{
  m_last_raw_value = value;

  for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    value = (*i)->filter(value, m_min, m_max);
  }

  if (m_last_send_value != value)
  {
    m_last_send_value = value;
    m_handler->send(uinput, value);
  }
}

void
AxisEvent::update(UInput& uinput, int msec_delta)
{
  for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  m_handler->update(uinput, msec_delta);

  send(uinput, m_last_raw_value);
}

void
AxisEvent::set_axis_range(int min, int max)
{
  m_min = min;
  m_max = max;
  m_handler->set_axis_range(min, max);
}

std::string
AxisEvent::str() const
{
  return m_handler->str();
}

AxisEventHandler::AxisEventHandler() :
  m_min(-1),
  m_max(+1)
{
}

void
AxisEventHandler::set_axis_range(int min, int max)
{
  m_min = min;
  m_max = max;
}

/* EOF */
