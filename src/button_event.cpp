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
#include "uinput_deviceid.hpp"

ButtonEvent
ButtonEvent::create(int type, int code)
{
  return ButtonEvent::create(DEVICEID_AUTO, type, code);
}

ButtonEvent
ButtonEvent::create(int device_id, int type, int code)
{
  ButtonEvent ev;

  ev.device_id = device_id;
  ev.type = type;
  ev.code = code;

  switch (type)
  {
    case EV_REL:
      ev.rel.repeat = 100;
      ev.rel.value  = 3;
      break;

    case EV_ABS:
      throw std::runtime_error("Using EV_ABS for ButtonEvent is currently not supported");
      ev.abs.value  = 1;
      break;

    case EV_KEY:
      ev.key.modifier[0] = -1;
      break;

    case -1:
      break;

    default:
      assert(!"This should never be reached");
  }

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
      std::string event_str;
      split_event_name(*i, &event_str, &ev.device_id);

      boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);

      std::vector<std::string> events;
      tokenizer ev_tokens(event_str, plus_sep);
      for(tokenizer::iterator k = ev_tokens.begin(); k != ev_tokens.end(); ++k)
      {
        std::cout << "XXX " << *k << std::endl;
        events.push_back(*k);
      }

      assert(!events.empty()); // HACK

      int type, code;
      if (!str2event(*events.begin(), type, code))
      {
        throw std::runtime_error("Couldn't convert '" + str + "' to ButtonEvent");
      }
      else
      {
        // create the event via function call to get proper default values
        ev = ButtonEvent::create(ev.device_id, type, code);

        int k = 0;
        for(std::vector<std::string>::iterator m = events.begin() + 1; m != events.end(); ++m)
        {
          str2event(*m, type, code);
          ev.key.modifier[k] = code;
          k += 1;
          if (k >= MAX_MODIFIER)
            break;
        }
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

bool
ButtonEvent::is_valid() const
{
  return device_id != DEVICEID_INVALID && type != -1 && code != -1;
}

/* EOF */
