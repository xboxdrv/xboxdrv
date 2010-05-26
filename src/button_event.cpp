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

#include "button_event.hpp"
#include "evdev_helper.hpp"
#include "uinput.hpp"
#include "uinput_deviceid.hpp"

ButtonEvent
ButtonEvent::invalid()
{
  ButtonEvent ev;
  ev.type = -1;
  return ev;
}

ButtonEvent
ButtonEvent::create_btn(int code)
{
  ButtonEvent ev;
  ev.type = EV_KEY;
  std::fill_n(ev.key.codes, MAX_MODIFIER + 1, UIEvent::invalid());
  ev.key.codes[0] = UIEvent::create(DEVICEID_AUTO, EV_KEY, code);
  return ev;
}

ButtonEvent
ButtonEvent::create_btn()
{
  ButtonEvent ev;
  ev.type = EV_KEY;
  std::fill_n(ev.key.codes, MAX_MODIFIER + 1, UIEvent::invalid());
  return ev;
}

ButtonEvent
ButtonEvent::create_rel(int code)
{
  ButtonEvent ev;
  ev.type       = EV_REL;
  ev.rel.code   = UIEvent::create(DEVICEID_AUTO, EV_REL, code);
  ev.rel.repeat = 100;
  ev.rel.value  = 3;
  return ev;
}

ButtonEvent
ButtonEvent::from_string(const std::string& str)
{
  ButtonEvent ev;
  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  int j = 0;
  tokenizer tokens(str, sep);
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    if (j == 0)
    {
      switch(get_event_type(*i))
      {
        case EV_KEY: {
          ev = ButtonEvent::create_btn();

          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
          {
            // create the event via function call to get proper default values
            ev.key.codes[k] = str2key_event(*m);
          }
          break;
        }

        case EV_REL:
          ev = ButtonEvent::create_rel(-1); // FIXME: Hack
          ev.rel.code = str2rel_event(*i);
          break;

        case EV_ABS:
          // FIXME: Need magic to detect min/max of the axis
          break;
      }
    }
    else
    {
      switch (ev.type)
      {
        case EV_REL:
          switch(j) {
            case 1: ev.rel.value  = boost::lexical_cast<int>(*i); break;
            case 2: ev.rel.repeat = boost::lexical_cast<int>(*i); break;
          }
          break;
      }
    }
  }

  return ev;
}

void
ButtonEvent::init(uInput& uinput) const
{
  assert(is_valid());

  switch(type)
  {
    case EV_KEY:
      for(int i = 0; key.codes[i].is_valid(); ++i)
      {
        uinput.create_uinput_device(key.codes[i].device_id);
        uinput.add_key(key.codes[i].device_id, key.codes[i].code);
      }
      break;

    case EV_REL:
      uinput.create_uinput_device(rel.code.device_id);
      uinput.get_uinput(rel.code.device_id)->add_rel(rel.code.code);
      break;

    case EV_ABS:
      uinput.create_uinput_device(abs.code.device_id);
      break;

    default:
      assert(!"ButtonEvent::init(): never reached");
  }
}

void
ButtonEvent::send(uInput& uinput, bool value) const
{
  assert(is_valid());

  switch(type)
  {
    case EV_KEY:
      for(int i = 0; key.codes[i].is_valid(); ++i)
      {
        uinput.send_key(key.codes[i].device_id, key.codes[i].code, value);
      }
      break;

    case EV_REL:
      if (value)
      {
        uinput.send_rel_repetitive(rel.code, rel.value, rel.repeat);
      }
      else
      {
        uinput.send_rel_repetitive(rel.code, rel.value, -1);
      }
      break;


    case EV_ABS:
      if (value)
      {
        uinput.get_uinput(abs.code.device_id)->send(EV_ABS, abs.code.code, abs.value);
      }
      break;
  }
}

bool
ButtonEvent::is_valid() const
{
  return type != -1;
}

/* EOF */
