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

#include <linux/input.h>
#include <assert.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <stdlib.h>
#include <iostream>

#include "axis_event.hpp"
#include "evdev_helper.hpp"
#include "uinput.hpp"
#include "uinput_deviceid.hpp"

AxisEventPtr
AxisEvent::invalid() 
{ 
  return AxisEventPtr();
}

AxisEventPtr
AxisEvent::create_abs(int device_id, int code, int min, int max, int fuzz, int flat)
{
  return AxisEventPtr(new AbsAxisEvent(device_id, code, min, max, fuzz, flat));
}

AxisEventPtr
AxisEvent::create_rel(int device_id, int code, int repeat, float value)
{
  return AxisEventPtr(new RelAxisEvent(device_id, code, repeat, value));
}
  
AxisEventPtr
AxisEvent::from_string(const std::string& str)
{
  AxisEventPtr ev;

  switch (get_event_type(str))
  {
    case EV_ABS:
      ev = AbsAxisEvent::from_string(str);
      break;

    case EV_REL:
      ev = RelAxisEvent::from_string(str);
      break;

    case EV_KEY:
      ev = KeyAxisEvent::from_string(str);
      break;

    case -1:
      std::cout << "--------- invalid --------------" << std::endl;
      ev = invalid();
      break;

    default:
      assert(!"AxisEvent::from_string(): should never be reached");
  }

  //std::cout << "AxisEvent::from_string():\n  in:  " << str << "\n  out: " << ev->str() << std::endl;

  return ev;
}

AxisEvent::AxisEvent() :
  m_filters()
{
}

void
AxisEvent::update(uInput& uinput, int msec_delta)
{
  /*
    for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
    {
    value = (*i)->filter(old_value, value);
    }
  */
}

void
AxisEvent::set_filters(const std::vector<AxisFilterPtr>& filters)
{
  m_filters = filters;
}

AxisEventPtr
RelAxisEvent::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  boost::shared_ptr<RelAxisEvent> ev(new RelAxisEvent);

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        ev->m_code = str2rel_event(*i);
        break;

      case 1:
        ev->m_value = boost::lexical_cast<int>(*i); 
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

  return ev;
}

RelAxisEvent::RelAxisEvent()
{
  m_code = UIEvent::invalid();
  m_value  = 5;
  m_repeat = 10;
}

RelAxisEvent::RelAxisEvent(int device_id, int code, int repeat, float value)
{
  m_code   = UIEvent::create(device_id, EV_REL, code);
  m_value  = value;
  m_repeat = repeat;
}

void
RelAxisEvent::init(uInput& uinput) const
{
  uinput.create_uinput_device(m_code.device_id);
  uinput.add_rel(m_code.device_id, m_code.code);
}

void
RelAxisEvent::send(uInput& uinput, int old_value, int value) const
{
  // FIXME: Need to know the min/max of value
  int v = m_value * value / 32767;
  if (v == 0)
    uinput.send_rel_repetitive(m_code, v, -1);
  else
    uinput.send_rel_repetitive(m_code, v, m_repeat);
}

void
RelAxisEvent::update(uInput& uinput, int msec_delta)
{
}

std::string
RelAxisEvent::str() const
{
  std::ostringstream out;
  out << m_code.device_id << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}

AxisEventPtr
AbsAxisEvent::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  
  int j = 0;
  int code = -1;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        code = str2abs(*i);
        break;

      default: 
        throw std::runtime_error("AxisEvent::abs_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEvent::abs_from_string(): at least one argument required: " + str);
  }
  else if (j > 1)
  {
    throw std::runtime_error("AxisEvent::abs_from_string(): invalid extra arguments in " + str);
  }
  else
  {
    AxisEventPtr ev = create_abs(DEVICEID_AUTO, code, -1, -1, 0, 0);
    return ev;
  }
}

AbsAxisEvent::AbsAxisEvent()
{
  m_code = UIEvent::invalid();
  m_min  = -32768; // FIXME: this must be properly set
  m_max  =  32767;
  m_fuzz = 0;
  m_flat = 0;
}

AbsAxisEvent::AbsAxisEvent(int device_id, int code, int min, int max, int fuzz, int flat)
{
  m_code  = UIEvent::create(device_id, EV_ABS, code);
  m_min   = min;
  m_max   = max;
  m_fuzz  = fuzz;
  m_flat  = flat;
}

void
AbsAxisEvent::set_axis_range(int min, int max)
{
  m_min = min;
  m_max = max;
}

void
AbsAxisEvent::init(uInput& uinput) const
{
  uinput.create_uinput_device(m_code.device_id);
  uinput.add_abs(m_code.device_id, m_code.code, 
                 m_min, m_max, m_fuzz, m_flat);
}

void
AbsAxisEvent:: send(uInput& uinput, int old_value, int value) const
{
  for(std::vector<AxisFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    value = (*i)->filter(old_value, value);
  }

  uinput.get_uinput(m_code.device_id)->send(EV_ABS, m_code.code, value);
}
 
void
AbsAxisEvent::update(uInput& uinput, int msec_delta)
{
}

std::string
AbsAxisEvent::str() const
{
  std::ostringstream out;
  out << m_code.device_id << "-" << m_code.code << ":" 
      << m_min << ":" << m_max << ":" 
      << m_fuzz << ":" << m_flat;
  return out.str();
}

AxisEventPtr
KeyAxisEvent::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  boost::shared_ptr<KeyAxisEvent> ev(new KeyAxisEvent);

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
          tokenizer ev_tokens(*i, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
          {
            ev->m_down_codes[k] = str2key_event(*m);
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

  return ev; 
}

KeyAxisEvent::KeyAxisEvent()
{
  std::fill_n(m_up_codes,   MAX_MODIFIER+1, UIEvent::invalid());
  std::fill_n(m_down_codes, MAX_MODIFIER+1, UIEvent::invalid());
  m_threshold = 8000;
}

void
KeyAxisEvent::init(uInput& uinput) const
{
  for(int i = 0; m_up_codes[i].is_valid(); ++i)
  {
    uinput.create_uinput_device(m_up_codes[i].device_id);
    uinput.add_key(m_up_codes[i].device_id, m_up_codes[i].code);
  }

  for(int i = 0; m_down_codes[i].is_valid(); ++i)
  {
    uinput.create_uinput_device(m_down_codes[i].device_id);
    uinput.add_key(m_down_codes[i].device_id, m_down_codes[i].code);
  }

}

void
KeyAxisEvent::send(uInput& uinput, int old_value, int value) const
{
  if (::abs(old_value) <  m_threshold &&
      ::abs(value)     >= m_threshold)
  { // entering bigger then threshold zone
    if (value < 0)
    {
      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_down_codes[i].device_id, m_down_codes[i].code, false);

      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_up_codes[i].device_id, m_up_codes[i].code, true);
    }
    else // (value > 0)
    { 
      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_down_codes[i].device_id, m_down_codes[i].code, true);

      for(int i = 0; m_up_codes[i].is_valid(); ++i)
        uinput.send_key(m_up_codes[i].device_id, m_up_codes[i].code, false);
    }
  }
  else if (::abs(old_value) >= m_threshold &&
           ::abs(value)     <  m_threshold)
  { // entering zero zone
    for(int i = 0; m_up_codes[i].is_valid(); ++i)
      uinput.send_key(m_down_codes[i].device_id, m_down_codes[i].code, false);

    for(int i = 0; m_up_codes[i].is_valid(); ++i)
      uinput.send_key(m_up_codes[i].device_id, m_up_codes[i].code, false);
  }
}

void
KeyAxisEvent::update(uInput& uinput, int msec_delta)
{
}

std::string
KeyAxisEvent::str() const
{
  std::ostringstream out;
  for(int i = 0; m_up_codes[i].is_valid();)
  {
    out << m_up_codes[i].device_id << "-" << m_up_codes[i].code;

    ++i;
    if (m_up_codes[i].is_valid())
      out << "+";
  }
      
  out << ":";

  for(int i = 0; m_down_codes[i].is_valid();)
  {
    out << m_down_codes[i].device_id << "-" << m_down_codes[i].code;

    ++i;
    if (m_down_codes[i].is_valid())
      out << "+";
  }

  out << ":" << m_threshold;

  return out.str();
}

/* EOF */
