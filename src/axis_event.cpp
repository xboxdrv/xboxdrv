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

#include "axis_event.hpp"
#include "evdev_helper.hpp"

AxisEvent 
AxisEvent::create(int type, int code, int fuzz, int flat)
{
  AxisEvent ev;
  ev.type = type;
  ev.code = code;
  
  switch (type)
    {
      case EV_REL:
        ev.rel.repeat = 10;
        ev.rel.value  = 5;
        break;

      case EV_ABS:
        ev.abs.fuzz = fuzz;
        ev.abs.flat = flat;
        break;

      case EV_KEY:
        ev.key.secondary_code = code;
        ev.key.threshold      = 8000;
        break;

      case -1:
        break;
        
      default:
        assert(!"This should never be reached");
    }

  return ev;
}

AxisEvent
AxisEvent::from_string(const std::string& str)
{
  AxisEvent ev;

  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokenizer(str, sep);

  std::vector<std::string> tokens;
  std::copy(tokenizer.begin(), tokenizer.end(), std::back_inserter(tokens));

  int j = 0;
  for(std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
    {
      if (j == 0)
        {
          int type, code;
          if (!str2event(*i, type, code))
            {
              throw std::runtime_error("Couldn't convert '" + str + "' to AxisEvent");
            }
          else
            {
              ev = AxisEvent::create(type, code);
            }
        }
      else
        {
          switch (ev.type)
            {
              case EV_ABS:
                break;

              case EV_REL:
                switch(j) {
                  case 1:  ev.rel.value  = boost::lexical_cast<int>(*i); break;
                  case 2:  ev.rel.repeat = boost::lexical_cast<int>(*i); break;
                }
                break;

              case EV_KEY:
                switch(j) {
                  case 1: 
                    { 
                      int type;
                      str2event(*i, type, ev.key.secondary_code);
                      assert(type == EV_KEY);
                    }
                    break;
                  case 2: ev.key.threshold = boost::lexical_cast<int>(*i); break;
                }
                break;
            }
        }
    }
    
  return ev;
}

/* EOF */
