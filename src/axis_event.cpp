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

#include "evdev_helper.hpp"
#include "log.hpp"
#include "helper.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

AxisEventPtr
AxisEvent::invalid() 
{ 
  return AxisEventPtr();
}

AxisEventPtr
AxisEvent::create_abs(int device_id, int code, int min, int max, int fuzz, int flat)
{
  return AxisEventPtr(new AxisEvent(new AbsAxisEventHandler(UIEvent::create(device_id, EV_ABS, code),
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

RelAxisEventHandler*
RelAxisEventHandler::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  std::auto_ptr<RelAxisEventHandler> ev(new RelAxisEventHandler);

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        ev->m_code = str2rel_event(*i);
        break;

      case 1:
        ev->m_value = boost::lexical_cast<float>(*i); 
        break;

      case 2:
        ev->m_repeat = boost::lexical_cast<int>(*i); 
        break;

      default: 
        throw std::runtime_error("AxisEvent::rel_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEvent::rel_from_string(): at least one argument required: " + str);
  }

  return ev.release();
}

RelAxisEventHandler::RelAxisEventHandler() :
  m_code(UIEvent::invalid()),
  m_value(5),
  m_repeat(10),
  m_stick_value(0.0f),
  m_rest_value(0.0f)
{
}

RelAxisEventHandler::RelAxisEventHandler(int device_id, int code, int repeat, float value) :
  m_code(UIEvent::create(device_id, EV_REL, code)),
  m_value(value),
  m_repeat(repeat),
  m_stick_value(0.0f),
  m_rest_value(0.0f)
{
}

void
RelAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_code.resolve_device_id(slot, extra_devices);
  uinput.add_rel(m_code.get_device_id(), m_code.code);
}

void
RelAxisEventHandler::send(UInput& uinput, int value)
{
  if (value < 0)
    m_stick_value = value / static_cast<float>(-m_min);
  else
    m_stick_value = value / static_cast<float>(m_max);

  if (m_repeat != -1)
  { 
    // regular old style sending of REL events
    float v = m_value * m_stick_value;

    if (v == 0)
      uinput.send_rel_repetitive(m_code, v, -1);
    else
      uinput.send_rel_repetitive(m_code, v, m_repeat);
  }
}

void
RelAxisEventHandler::update(UInput& uinput, int msec_delta)
{
  if (m_repeat == -1 && m_stick_value != 0.0f)
  {
    // new and improved REL style event sending

    float rel_value = m_stick_value * m_value * static_cast<float>(msec_delta) / 1000.0f;

    // keep track of the rest that we lose when converting to integer
    rel_value += m_rest_value;
    m_rest_value = rel_value - truncf(rel_value);

    uinput.send_rel(m_code.get_device_id(), m_code.code, static_cast<int>(rel_value));
  }
}

std::string
RelAxisEventHandler::str() const
{
  std::ostringstream out;
  out << m_code.get_device_id() << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}


RelRepeatAxisEventHandler*
RelRepeatAxisEventHandler::from_string(const std::string& str)
{
  // split string at ':'
  boost::tokenizer<boost::char_separator<char> > 
    tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args;
  std::copy(tokens.begin(), tokens.end(), std::back_inserter(args));

  if (args.size() == 3)
  {
    return new RelRepeatAxisEventHandler(str2rel_event(args[0]),
                                         boost::lexical_cast<int>(args[1]),
                                         boost::lexical_cast<float>(args[2]));
  }
  else
  {
    raise_exception(std::runtime_error, "must have three arguments");
  }
}

RelRepeatAxisEventHandler::RelRepeatAxisEventHandler(const UIEvent& code, int value, int repeat) :
  m_code(code),
  m_value(value),
  m_repeat(repeat),
  m_stick_value(0),
  m_timer(0)
{  
}

void
RelRepeatAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_code.resolve_device_id(slot, extra_devices);
  uinput.add_rel(m_code.get_device_id(), m_code.code);
}

void
RelRepeatAxisEventHandler::send(UInput& uinput, int value)
{
  if (value < 0)
  {
    m_stick_value = value / static_cast<float>(-m_min);
  }
  else
  {
    m_stick_value = value / static_cast<float>(m_max);
  }

  // reset timer when in center position
  if (value == 0)
  {
    m_timer = 0;
  }
}

void
RelRepeatAxisEventHandler::update(UInput& uinput, int msec_delta)
{
  // time ticks slower depending on how fr the stick is moved
  m_timer += msec_delta * fabsf(m_stick_value);

  while(m_timer > m_repeat)
  {
    if (m_stick_value < 0)
    {
      uinput.send_rel(m_code.get_device_id(), m_code.code, -m_value);
    }
    else
    {
      uinput.send_rel(m_code.get_device_id(), m_code.code, m_value);
    }
    
    m_timer -= m_repeat;
  }
}

std::string
RelRepeatAxisEventHandler::str() const
{
  std::ostringstream out;
  out << "rel-repeat:" << m_value << ":" << m_repeat;
  return out.str();
}

AbsAxisEventHandler*
AbsAxisEventHandler::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  
  int j = 0;
  UIEvent code = UIEvent::invalid();
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        code = str2abs_event(*i);
        break;

      default: 
        throw std::runtime_error("AxisEventHandlers::abs_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEventHandler::abs_from_string(): at least one argument required: " + str);
  }
  else if (j > 1)
  {
    throw std::runtime_error("AxisEventHandler::abs_from_string(): invalid extra arguments in " + str);
  }
  else
  {
    return new AbsAxisEventHandler(code, -1, -1, 0, 0);
  }
}

AbsAxisEventHandler::AbsAxisEventHandler() :
  m_code(UIEvent::invalid()),
  m_fuzz(0),
  m_flat(0)
{
}

AbsAxisEventHandler::AbsAxisEventHandler(const UIEvent& code, int min, int max, int fuzz, int flat) :
  m_code(code),
  m_fuzz(fuzz),
  m_flat(flat)
{
  set_axis_range(min, max);
}

void
AbsAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_code.resolve_device_id(slot, extra_devices);
  uinput.add_abs(m_code.get_device_id(), m_code.code, 
                 m_min, m_max, m_fuzz, m_flat);
}

void
AbsAxisEventHandler::send(UInput& uinput, int value)
{
  uinput.send_abs(m_code.get_device_id(), m_code.code, value);
}
 
void
AbsAxisEventHandler::update(UInput& uinput, int msec_delta)
{
}

std::string
AbsAxisEventHandler::str() const
{
  std::ostringstream out;
  out << m_code.get_device_id() << "-" << m_code.code << ":" 
      << m_min << ":" << m_max << ":" 
      << m_fuzz << ":" << m_flat;
  return out.str();
}

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
  m_threshold(8000)
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
KeyAxisEventHandler::send(UInput& uinput, int value)
{
  if (::abs(m_old_value) <  m_threshold &&
      ::abs(value)       >= m_threshold)
  { // entering bigger then threshold zone
    if (value < 0)
    {
      for(int i = 0; m_down_codes[i].is_valid(); ++i)
        uinput.send_key(m_down_codes[i].get_device_id(), m_down_codes[i].code, false);

      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_up_codes[i].get_device_id(), m_up_codes[i].code, true);
    }
    else // (value > 0)
    { 
      for(int i = 0; m_down_codes[i].is_valid(); ++i)
        uinput.send_key(m_down_codes[i].get_device_id(), m_down_codes[i].code, true);

      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_up_codes[i].get_device_id(), m_up_codes[i].code, false);
    }
  }
  else if (::abs(m_old_value) >= m_threshold &&
           ::abs(value)       <  m_threshold)
  { // entering zero zone
    for(int i = 0; m_down_codes[i].is_valid(); ++i)
      uinput.send_key(m_down_codes[i].get_device_id(), m_down_codes[i].code, false);

    for(int i = 0; m_up_codes[i].is_valid(); ++i)
      uinput.send_key(m_up_codes[i].get_device_id(), m_up_codes[i].code, false);
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
