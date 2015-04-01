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

#include "button_event.hpp"

#include <boost/tokenizer.hpp>
#include <errno.h>
#include <fstream>

#include "evdev_helper.hpp"
#include "log.hpp"
#include "path.hpp"

ButtonEvent::ButtonEvent(ButtonEventHandler* handler) :
  m_last_send_state(false),
  m_last_raw_state(false),
  m_handler(handler),
  m_filters()
{
}

void
ButtonEvent::add_filters(const std::vector<ButtonFilterPtr>& filters)
{
  std::copy(filters.begin(), filters.end(), std::back_inserter(m_filters));
}

void
ButtonEvent::add_filter(ButtonFilterPtr filter)
{
  m_filters.push_back(filter);
}

void
ButtonEvent::send(bool raw_state)
{
  m_last_raw_state = raw_state;
  bool filtered_state = raw_state;

  // filter values
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    filtered_state = (*i)->filter(filtered_state);
  }

  if (m_last_send_state == filtered_state)
  {
    // button state has not changed, so do not send events
  }
  else
  {
    m_last_send_state = filtered_state;
    m_handler->send(m_last_send_state);
  }
}

void
ButtonEvent::send_clear()
{
  m_last_send_state = false;
  m_handler->send_clear();
}

void
ButtonEvent::update(int msec_delta)
{
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  m_handler->update(msec_delta);

  send(m_last_raw_state);
}

std::string
ButtonEvent::str() const
{
  return m_handler->str();
}

/* EOF */
