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
AxisEvent::send(int value)
{
  m_last_raw_value = value;

  for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    value = (*i)->filter(value, m_min, m_max);
  }

  if (m_last_send_value != value)
  {
    m_last_send_value = value;
    m_handler->send(value);
  }
}

void
AxisEvent::update(int msec_delta)
{
  for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  m_handler->update(msec_delta);

  send(m_last_raw_value);
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
