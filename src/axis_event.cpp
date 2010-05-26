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

AxisEvent
AxisEvent::invalid() 
{ 
  AxisEvent ev;
  ev.type = -1;
  return ev;
}

AxisEvent
AxisEvent::create_abs(int device_id, int code, int min, int max, int fuzz, int flat)
{
  AxisEvent ev;
  ev.type      = EV_ABS;
  ev.abs.code  = UIEvent::create(device_id, EV_ABS, code);
  ev.abs.min   = min;
  ev.abs.max   = max;
  ev.abs.fuzz  = fuzz;
  ev.abs.flat  = flat;
  return ev;
}

AxisEvent
AxisEvent::create_rel(int device_id, int code, int repeat, float value)
{
  AxisEvent ev;
  ev.type       = EV_REL;
  ev.rel.code   = UIEvent::create(device_id, EV_REL, code);
  ev.rel.value  = value;
  ev.rel.repeat = repeat;
  return ev;  
}

AxisEvent
AxisEvent::create_key()
{
  AxisEvent ev;
  ev.type = EV_KEY;
  std::fill_n(ev.key.up_codes,   MAX_MODIFIER+1, UIEvent::invalid());
  std::fill_n(ev.key.down_codes, MAX_MODIFIER+1, UIEvent::invalid());
  ev.key.threshold = 8000;
  return ev;
}

AxisEvent
AxisEvent::create_rel()
{
  AxisEvent ev;
  ev.type = EV_REL;
  ev.rel.code = UIEvent::invalid();
  ev.rel.value  = 5;
  ev.rel.repeat = 10;
  return ev;
}
  
AxisEvent
AxisEvent::create_abs()
{
  AxisEvent ev;
  ev.type = EV_ABS;
  ev.abs.code = UIEvent::invalid();
  ev.abs.min  = -32768; // FIXME: this must be properly set
  ev.abs.max  =  32767;
  ev.abs.fuzz = 0;
  ev.abs.flat = 0;
  return ev;
}

AxisEvent
AxisEvent::from_string(const std::string& str)
{
  AxisEvent ev;

  switch (get_event_type(str))
  {
    case EV_ABS:
      ev = abs_from_string(str);
      break;

    case EV_REL:
      ev = rel_from_string(str);
      break;

    case EV_KEY:
      return key_from_string(str);

    case -1:
      ev = invalid();
      break;

    default:
      assert(!"AxisEvent::from_string(): should never be reached");
  }

  std::cout << "AxisEvent::from_string():\n  in:  " << str << "\n  out: " << ev.str() << std::endl;

  return ev;
}

AxisEvent
AxisEvent::abs_from_string(const std::string& str)
{
  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  AxisEvent ev = create_key();

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        break;

      default: 
        throw std::runtime_error("AxisEvent::abs_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEvent::abs_from_string(): at least one argument required: " + str);
  }

  return ev;
}

AxisEvent
AxisEvent::rel_from_string(const std::string& str)
{
  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  AxisEvent ev = create_rel();

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        ev.rel.code = str2rel_event(*i);
        break;

      case 1:
        ev.rel.value = boost::lexical_cast<int>(*i); 
        break;

      case 2:
        ev.rel.repeat = boost::lexical_cast<int>(*i); 
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

AxisEvent
AxisEvent::key_from_string(const std::string& str)
{
  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  AxisEvent ev = create_key();

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        ev.key.up_codes[0] = str2key_event(*i);
        break;

      case 1:
        ev.key.down_codes[0] = str2key_event(*i);
        break;

      case 2:
        ev.key.threshold = boost::lexical_cast<int>(*i);
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

AxisEvent::AxisEvent() :
  type(-1)
{
}

bool
AxisEvent::is_valid() const
{
  return type != -1;
}

void
AxisEvent::init(uInput& uinput) const
{
  assert(is_valid());

  switch(type)
  {
    case EV_KEY:
      for(int i = 0; key.up_codes[i].is_valid(); ++i)
      {
        uinput.create_uinput_device(key.up_codes[i].device_id);
        uinput.add_key(key.up_codes[i].device_id, key.up_codes[i].code);
      }

      for(int i = 0; key.down_codes[i].is_valid(); ++i)
      {
        uinput.create_uinput_device(key.down_codes[i].device_id);
        uinput.add_key(key.down_codes[i].device_id, key.down_codes[i].code);
      }
      break;

    case EV_REL:
      uinput.create_uinput_device(rel.code.device_id);
      uinput.add_rel(rel.code.device_id, rel.code.code);
      break;

    case EV_ABS:
      uinput.create_uinput_device(abs.code.device_id);
      uinput.add_abs(abs.code.device_id, abs.code.code, 
                     abs.min, abs.max, abs.fuzz, abs.flat);
      break;
  }
}

void
AxisEvent::send(uInput& uinput, int old_value, int value) const
{
  assert(is_valid());

  switch(type)
  {
    case EV_ABS:
      uinput.get_uinput(abs.code.device_id)->send(type, abs.code.code, value);
      break;
      
    case EV_REL:
      {
        // FIXME: Need to know the min/max of value
        int v = rel.value * value / 32767;
        if (v == 0)
          uinput.send_rel_repetitive(rel.code, v, -1);
        else
          uinput.send_rel_repetitive(rel.code, v, rel.repeat);
      }
      break;

    case EV_KEY:
      if (::abs(old_value) <  key.threshold &&
          ::abs(value)     >= key.threshold)
      { // entering bigger then threshold zone
        if (value < 0)
        {
          for(int i = 0; key.up_codes[i].is_valid(); ++i)
            uinput.send_key(key.down_codes[i].device_id, key.down_codes[i].code, false);

          for(int i = 0; key.up_codes[i].is_valid(); ++i)
            uinput.send_key(key.up_codes[i].device_id, key.up_codes[i].code, true);
        }
        else // (value > 0)
        { 
          for(int i = 0; key.up_codes[i].is_valid(); ++i)
            uinput.send_key(key.down_codes[i].device_id, key.down_codes[i].code, true);

          for(int i = 0; key.up_codes[i].is_valid(); ++i)
            uinput.send_key(key.up_codes[i].device_id, key.up_codes[i].code, false);
        }
      }
      else if (::abs(old_value) >= key.threshold &&
               ::abs(value)     <  key.threshold)
      { // entering zero zone
        for(int i = 0; key.up_codes[i].is_valid(); ++i)
          uinput.send_key(key.down_codes[i].device_id, key.down_codes[i].code, false);

        for(int i = 0; key.up_codes[i].is_valid(); ++i)
          uinput.send_key(key.up_codes[i].device_id, key.up_codes[i].code, false);
      }
      break;
  }
}

std::string
AxisEvent::str() const
{
  std::ostringstream out;
  switch(type)
  {
    case EV_ABS:
      out << abs.code.device_id << "-" << abs.code.code << ":" << abs.min << ":" << abs.max << ":" << abs.fuzz << ":" << abs.flat;
      break;

    case EV_REL:
      out << rel.code.device_id << "-" << rel.code.code << ":" << rel.value << ":" << rel.repeat;
      break;

    case EV_KEY:
      for(int i = 0; key.up_codes[i].is_valid();)
      {
        out << key.up_codes[i].device_id << "-" << key.up_codes[i].code;

        ++i;
        if (key.up_codes[i].is_valid())
          out << "+";
      }
      
      out << ":";

      for(int i = 0; key.down_codes[i].is_valid();)
      {
        out << key.down_codes[i].device_id << "-" << key.down_codes[i].code;

        ++i;
        if (key.down_codes[i].is_valid())
          out << "+";
      }

      out << ":" << key.threshold;
      break;
  }
  return out.str();
}

/* EOF */
