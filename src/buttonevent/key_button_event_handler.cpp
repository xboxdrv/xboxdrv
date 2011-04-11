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

          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end() && k < MAX_MODIFIER; ++m, ++k)
          {
            ev->m_codes[k] = str2key_event(*m);
          }
        }
        break;

      case 1:
        {
          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end() && k < MAX_MODIFIER; ++m, ++k)
          {
            ev->m_secondary_codes[k] = str2key_event(*m);
          }

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
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
}

KeyButtonEventHandler::KeyButtonEventHandler(int device_id, int code) :
  m_state(false),
  m_codes(),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  m_codes[0] = UIEvent::create(device_id, EV_KEY, code);
}

void
KeyButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  for(int i = 0; m_codes[i].is_valid(); ++i)
  {
    m_codes[i].resolve_device_id(slot, extra_devices);
    uinput.add_key(m_codes[i].get_device_id(), m_codes[i].code);
  }

  if (m_hold_threshold)
  {
    for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
    {
      m_secondary_codes[i].resolve_device_id(slot, extra_devices);
      uinput.add_key(m_secondary_codes[i].get_device_id(), m_secondary_codes[i].code);
    }
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
      // FIXME: should handle key releases in reverse order
      for(int i = 0; m_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_codes[i].get_device_id(), m_codes[i].code, m_state);
      }
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
          for(int i = 0; m_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_codes[i].get_device_id(), m_codes[i].code, true);
          }
          // FIXME: should do this in reverse order
          for(int i = 0; m_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_codes[i].get_device_id(), m_codes[i].code, false);
          }
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
          // FIXME: should do in reverse
          for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_secondary_codes[i].get_device_id(), m_secondary_codes[i].code, false);
          }
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
      for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_secondary_codes[i].get_device_id(), m_secondary_codes[i].code, true);
      }
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
  for(int i = 0; m_codes[i].is_valid();)
  {
    out << m_codes[i].get_device_id() << "-" << m_codes[i].code;

    ++i;
    if (m_codes[i].is_valid())
      out << "+";
  }
  return out.str();
}

/* EOF */
