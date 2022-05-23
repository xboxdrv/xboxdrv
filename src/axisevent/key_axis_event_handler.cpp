/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include <uinpp/multi_device.hpp>
#include <uinpp/event_sequence.hpp>
#include <uinpp/from.hpp>

#include "evdev_helper.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

KeyAxisEventHandler*
KeyAxisEventHandler::from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                 const std::string& str)
{
  auto tokens = string_split(str, ":");

  uinpp::EventSequence up_codes;
  uinpp::EventSequence down_codes;
  float threshold = 0.25f;

  int j = 0;
  for(auto i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        {
          up_codes = uinpp::EventSequence_from_string(*i);
        }
        break;

      case 1:
        {
          if (is_float(*i))
          {
            // bit of hackery to handle simplified syntax for trigger button that don't need up/down events
            threshold = str2float(*i);
            down_codes = up_codes;
            up_codes.clear();
          }
          else
          {
            down_codes = uinpp::EventSequence_from_string(*i);
          }
        }
        break;

      case 2:
        threshold = str2float(*i);
        break;

      default:
        throw std::runtime_error("AxisEvent::key_from_string(): to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    throw std::runtime_error("AxisEvent::key_from_string(): at least one argument required: " + str);
  }

  return new KeyAxisEventHandler(uinput, slot, extra_devices,
                                 up_codes, down_codes, threshold);

}

KeyAxisEventHandler::KeyAxisEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                         uinpp::EventSequence up_codes,
                                         uinpp::EventSequence down_codes,
                                         float threshold) :
  m_old_value(0),
  m_up_codes(up_codes),
  m_down_codes(down_codes),
  m_threshold(threshold)
{
  m_up_codes.init(uinput, slot, extra_devices);
  m_down_codes.init(uinput, slot, extra_devices);
}

int
KeyAxisEventHandler::get_zone(float value) const
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
KeyAxisEventHandler::send(int value, int min, int max)
{
  int old_zone = get_zone(to_float(m_old_value, min, max));
  int zone     = get_zone(to_float(value, min, max));

  if (old_zone != zone)
  {
    // release the keys of the zone we leave
    if (old_zone == -1)
    {
      m_up_codes.send(false);
    }
    else if (old_zone == +1)
    {
      m_down_codes.send(false);
    }

    // press the keys of the zone we enter
    if (zone == +1)
    {
      m_down_codes.send(true);
    }
    else if (zone == -1)
    {
      m_up_codes.send(true);
    }
  }

  m_old_value = value;
}

void
KeyAxisEventHandler::update(int msec_delta)
{
}

std::string
KeyAxisEventHandler::str() const
{
  std::ostringstream out;
  out << m_up_codes.str() << ":" << m_down_codes.str() << ":" << m_threshold;
  return out.str();
}

/* EOF */
