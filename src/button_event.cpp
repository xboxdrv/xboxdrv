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

#include <assert.h>
#include <iostream>
#include <linux/input.h>
#include <stdexcept>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "helper.hpp"
#include "button_event.hpp"
#include "evdev_helper.hpp"
#include "uinput.hpp"
#include "uinput_deviceid.hpp"

ButtonEventPtr
ButtonEvent::invalid()
{
  return ButtonEventPtr();
}

ButtonEventPtr
ButtonEvent::create_abs(int code)
{
  return ButtonEventPtr(new AbsButtonEvent(code));
}

ButtonEventPtr
ButtonEvent::create_key(int code)
{
  return ButtonEventPtr(new KeyButtonEvent(code));
}

ButtonEventPtr
ButtonEvent::create_key()
{
  return ButtonEventPtr(new KeyButtonEvent);
}

ButtonEventPtr
ButtonEvent::create_rel(int code)
{
  return ButtonEventPtr(new RelButtonEvent(UIEvent::create(DEVICEID_AUTO, EV_REL, code)));
}

ButtonEventPtr
ButtonEvent::from_string(const std::string& str)
{
  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);

  if (token == "abs")
  {
    return AbsButtonEvent::from_string(str.substr(p+1));
  }
  else if (token == "rel")
  {
    return RelButtonEvent::from_string(str.substr(p+1));
  }
  else if (token == "key")
  {
    return KeyButtonEvent::from_string(str.substr(p+1));
  }
  else
  {
    // try to guess the type of event on the type of the first event code
    switch(get_event_type(token))
    {
      case EV_KEY: return KeyButtonEvent::from_string(str);
      case EV_REL: return RelButtonEvent::from_string(str);
      case EV_ABS: return AbsButtonEvent::from_string(str);
      case     -1: return ButtonEvent::invalid();
      default: assert(!"unknown type");
    }
  }
}

ButtonEvent::ButtonEvent() :
  m_filters()
{
}

void
ButtonEvent::set_filters(const std::vector<ButtonFilterPtr>& filters)
{
  m_filters = filters;
}

bool
ButtonEvent::apply_filter(bool value) const
{
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    value = (*i)->filter(value);
  }
  return value;
}

ButtonEventPtr
KeyButtonEvent::from_string(const std::string& str)
{
  //std::cout << " KeyButtonEvent::from_string: " << str << std::endl;

  boost::shared_ptr<KeyButtonEvent> ev;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {

    switch(idx)
    {
      case 0: 
        {
          ev.reset(new KeyButtonEvent());

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

  return ev;
}

KeyButtonEvent::KeyButtonEvent() :
  m_state(false),
  m_codes(),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
}

KeyButtonEvent::KeyButtonEvent(int code) :
  m_codes()
{
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  m_codes[0] = UIEvent::create(DEVICEID_AUTO, EV_KEY, code);
}

void
KeyButtonEvent::init(uInput& uinput) const
{
  for(int i = 0; m_codes[i].is_valid(); ++i)
  {
    uinput.create_uinput_device(m_codes[i].device_id);
    uinput.add_key(m_codes[i].device_id, m_codes[i].code);
  }

  if (m_hold_threshold)
  {
    for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
    {
      uinput.add_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code);
    }
  }
}

void
KeyButtonEvent::send(uInput& uinput, bool value)
{
  value = apply_filter(value);

  if (m_state != value)
  {
    m_state = value;

    if (m_hold_threshold == 0)
    {
      // FIXME: should handle key releases in reverse order
      for(int i = 0; m_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_codes[i].device_id, m_codes[i].code, m_state);
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
            uinput.send_key(m_codes[i].device_id, m_codes[i].code, true);
          }
          // FIXME: should do this in reverse order
          for(int i = 0; m_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_codes[i].device_id, m_codes[i].code, false);
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
            uinput.send_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code, false);
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
KeyButtonEvent::update(uInput& uinput, int msec_delta) 
{
  if (m_state && m_hold_threshold)
  {
    if (m_hold_counter < m_hold_threshold &&
        m_hold_counter + msec_delta >= m_hold_threshold)
    {
      // start sending the secondary events
      for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code, true);
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
KeyButtonEvent::str() const
{
  std::ostringstream out;
  for(int i = 0; m_codes[i].is_valid();)
  {
    out << m_codes[i].device_id << "-" << m_codes[i].code;

    ++i;
    if (m_codes[i].is_valid())
      out << "+";
  }
  return out.str();
}

ButtonEventPtr
AbsButtonEvent::from_string(const std::string& str)
{
  // FIXME: Need magic to detect min/max of the axis
  assert(!"not implemented");
}

AbsButtonEvent::AbsButtonEvent(int code) :
  m_code(UIEvent::invalid()),
  m_value()
{
  assert(!"Not implemented");
  // FIXME: Need magic to detect min/max of the axis
}

void
AbsButtonEvent::init(uInput& uinput) const
{
  uinput.create_uinput_device(m_code.device_id);
}

void
AbsButtonEvent::send(uInput& uinput, bool value)
{
  value = apply_filter(value);

  if (value)
  {
    uinput.get_uinput(m_code.device_id)->send(EV_ABS, m_code.code, m_value);
  }
}

std::string
AbsButtonEvent::str() const
{
  std::ostringstream out;
  out << "abs: " << m_code.device_id << "-" << m_code.code << ":" << m_value; 
  return out.str();
}

ButtonEventPtr
RelButtonEvent::from_string(const std::string& str)
{
  boost::shared_ptr<RelButtonEvent> ev;

  int idx = 0;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0:
        ev.reset(new RelButtonEvent(str2rel_event(*i)));
        break;

      case 1: 
        ev->m_value  = boost::lexical_cast<int>(*i);
        break;
        
      case 2: 
        ev->m_repeat = boost::lexical_cast<int>(*i); 
        break;
    }
  }

  return ev;
}

RelButtonEvent::RelButtonEvent(const UIEvent& code) :
  m_code(code),
  m_value(3),
  m_repeat(100)
{
}

void
RelButtonEvent::init(uInput& uinput) const
{
  uinput.create_uinput_device(m_code.device_id);
  uinput.get_uinput(m_code.device_id)->add_rel(m_code.code);
}

void
RelButtonEvent::send(uInput& uinput, bool value)
{
  value = apply_filter(value);

  if (value)
  {
    uinput.send_rel_repetitive(m_code, m_value, m_repeat);
  }
  else
  {
    uinput.send_rel_repetitive(m_code, m_value, -1);
  } 
}

std::string
RelButtonEvent::str() const
{
  std::ostringstream out;
  out << "rel:" << m_code.device_id << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}

/* EOF */
