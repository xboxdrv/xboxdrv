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
          int k = 0;
          tokenizer ev_tokens(*i, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
          {
            ev->m_up_codes[k] = str2key_event(*m);
          }
        }
        break;

      case 1:
        {
          if (is_number(*i))
          {
            // bit of hackery to handle simplified syntax for trigger button that don't need up/down events
            ev->m_threshold = boost::lexical_cast<int>(*i);

            for(int k = 0; ev->m_up_codes[k].is_valid(); ++k)
            {
              ev->m_down_codes[k] = ev->m_up_codes[k];
              ev->m_up_codes[k] = UIEvent::invalid();
            }
          }
          else
          {
            tokenizer ev_tokens(*i, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
            int k = 0;
            for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
            {
              ev->m_down_codes[k] = str2key_event(*m);
            }
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
  m_threshold(8000) // BUG: this doesn't work for triggers
{
  std::fill_n(m_up_codes,   MAX_MODIFIER+1, UIEvent::invalid());
  std::fill_n(m_down_codes, MAX_MODIFIER+1, UIEvent::invalid());
}

void
KeyAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  for(int i = 0; m_up_codes[i].is_valid(); ++i)
  {
    m_up_codes[i].resolve_device_id(slot, extra_devices);
    uinput.add_key(m_up_codes[i].get_device_id(), m_up_codes[i].code);
  }

  for(int i = 0; m_down_codes[i].is_valid(); ++i)
  {
    m_down_codes[i].resolve_device_id(slot, extra_devices);
    uinput.add_key(m_down_codes[i].get_device_id(), m_down_codes[i].code);
  }
}

void
KeyAxisEventHandler::send_up(UInput& uinput, int value)
{
  for(int i = 0; m_up_codes[i].is_valid(); ++i)
    uinput.send_key(m_up_codes[i].get_device_id(), m_up_codes[i].code, value);
}

void
KeyAxisEventHandler::send_down(UInput& uinput, int value)
{
  for(int i = 0; m_down_codes[i].is_valid(); ++i)
    uinput.send_key(m_down_codes[i].get_device_id(), m_down_codes[i].code, value);
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
      send_down(uinput, false);
    }
    else if (old_zone == +1)
    {
      send_up(uinput, false);
    }

    // press the keys of the zone we enter
    if (zone == +1)
    {
      send_up(uinput, true);
    }
    else if (zone == -1)
    {
      send_down(uinput, true);
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
  for(int i = 0; m_up_codes[i].is_valid();)
  {
    out << m_up_codes[i].get_device_id() << "-" << m_up_codes[i].code;

    ++i;
    if (m_up_codes[i].is_valid())
      out << "+";
  }
      
  out << ":";

  for(int i = 0; m_down_codes[i].is_valid();)
  {
    out << m_down_codes[i].get_device_id() << "-" << m_down_codes[i].code;

    ++i;
    if (m_down_codes[i].is_valid())
      out << "+";
  }

  out << ":" << m_threshold;

  return out.str();
}

/* EOF */
