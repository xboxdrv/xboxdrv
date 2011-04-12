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

#include "axisevent/rel_repeat_axis_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <math.h>

#include "evdev_helper.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

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

/* EOF */
