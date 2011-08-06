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

#include "key_axis_event_handler.hpp"

#include <boost/tokenizer.hpp>

#include "evdev_helper.hpp"
#include "helper.hpp"
#include "uinput.hpp"

KeyAxisEventHandler*
KeyAxisEventHandler::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  
  std::auto_ptr<KeyAxisEventHandler> ev(new KeyAxisEventHandler);

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        {
          ev->m_up_codes = UIEventSequence::from_string(*i);
        }
        break;

      case 1:
        {
          if (is_number(*i))
          {
            // bit of hackery to handle simplified syntax for trigger button that don't need up/down events
            ev->m_threshold = boost::lexical_cast<int>(*i);
            ev->m_down_codes = ev->m_up_codes;
            ev->m_up_codes.clear();
          }
          else
          {
            ev->m_down_codes = UIEventSequence::from_string(*i);
          }
        }
        break;
        
      case 2:
        ev->m_threshold = boost::lexical_cast<int>(*i);
        break;
        
      default: 
        throw std::runtime_error("AxisEvent::key_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEvent::key_from_string(): at least one argument required: " + str);
  }

  return ev.release();
}

KeyAxisEventHandler::KeyAxisEventHandler() :
  m_old_value(0),
  m_up_codes(),
  m_down_codes(),
  m_threshold(8000) // FIXME: this doesn't work for triggers
{
}

void
KeyAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_up_codes.init(uinput, slot, extra_devices);
  m_down_codes.init(uinput, slot, extra_devices);
}

void
KeyAxisEventHandler::send_up(UInput& uinput, int value)
{
  m_up_codes.send(uinput, value);
}

void
KeyAxisEventHandler::send_down(UInput& uinput, int value)
{
  m_down_codes.send(uinput, value);
}

int
KeyAxisEventHandler::get_zone(int value) const
{
  if (value >= m_threshold)
  {
    return +1;
  }
  else if (value <= -m_threshold)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

void
KeyAxisEventHandler::send(UInput& uinput, int value)
{
  int old_zone = get_zone(m_old_value);
  int zone     = get_zone(value);

  if (old_zone != zone)
  {
    // release the keys of the zone we leave
    if (old_zone == -1)
    {
      send_up(uinput, false);
    }
    else if (old_zone == +1)
    {
      send_down(uinput, false);
    }

    // press the keys of the zone we enter
    if (zone == +1)
    {
      send_down(uinput, true);
    }
    else if (zone == -1)
    {
      send_up(uinput, true);
    }
  }

  m_old_value = value;
}

void
KeyAxisEventHandler::update(UInput& uinput, int msec_delta)
{
}

std::string
KeyAxisEventHandler::str() const
{
  std::ostringstream out;
  out << m_up_codes.str() << ":" << m_down_codes.str() << ":" << m_threshold;
  return out.str();
}

/* EOF */
