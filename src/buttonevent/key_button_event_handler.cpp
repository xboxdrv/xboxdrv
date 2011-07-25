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

#include "buttonevent/key_button_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <linux/input.h>

#include "evdev_helper.hpp"
#include "uinput.hpp"

KeyButtonEventHandler*
KeyButtonEventHandler::from_string(const std::string& str)
{
  //std::cout << " KeyButtonEventHandler::from_string: " << str << std::endl;

  std::auto_ptr<KeyButtonEventHandler> ev;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0: 
        {
          ev.reset(new KeyButtonEventHandler());
          ev->m_codes = UIEventSequence::from_string(*i);
        }
        break;

      case 1:
        {
          ev->m_secondary_codes = UIEventSequence::from_string(*i);
          ev->m_hold_threshold = 250;
        }
        break;
        
      case 2:
        {
          ev->m_hold_threshold = boost::lexical_cast<int>(*i);
        }
        break;

      default:
        {
          std::ostringstream out;
          out << "to many arguments in '" << str << "'";
          throw std::runtime_error(out.str());
        }
        break;
    }
  }

  return ev.release();
}

KeyButtonEventHandler::KeyButtonEventHandler() :
  m_state(false),
  m_codes(),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
}

KeyButtonEventHandler::KeyButtonEventHandler(int device_id, int code) :
  m_state(false),
  m_codes(UIEvent::create(static_cast<uint16_t>(device_id), EV_KEY, code)),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
}

void
KeyButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_codes.init(uinput, slot, extra_devices);

  if (m_hold_threshold)
  {
    m_secondary_codes.init(uinput, slot, extra_devices);
  }
}

void
KeyButtonEventHandler::send(UInput& uinput, bool value)
{
  if (m_state != value)
  {
    m_state = value;

    if (m_hold_threshold == 0)
    {
      m_codes.send(uinput, m_state);
    }
    else
    {
      if (m_hold_counter < m_hold_threshold)
      {
        if (m_state)
        {
          // we are only sending events after release or when
          // hold_threshold is passed
        }
        else
        {
          // send both a press and release event after another, aka a "click"
          m_codes.send(uinput, true);
          m_codes.send(uinput, false);
        }
      }
      else
      {
        if (m_state)
        {
          // should never happen
        }
        else
        {
          m_secondary_codes.send(uinput, false);
        }
      }

      if (!m_state)
      {
        m_hold_counter = 0;
      }
    }
  }
}

void
KeyButtonEventHandler::update(UInput& uinput, int msec_delta) 
{
  if (m_state && m_hold_threshold)
  {
    if (m_hold_counter < m_hold_threshold &&
        m_hold_counter + msec_delta >= m_hold_threshold)
    {
      // start sending the secondary events
      m_secondary_codes.send(uinput, true);
      uinput.sync();
    }

    if (m_hold_counter < m_hold_threshold)
    {
      m_hold_counter += msec_delta;
    }
  }
}

std::string
KeyButtonEventHandler::str() const
{
  std::ostringstream out;
  out << m_codes.str() << ":" << m_secondary_codes.str() << ":" << m_hold_threshold;
  return out.str();
}

/* EOF */
