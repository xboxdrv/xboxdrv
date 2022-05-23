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

#include "axisevent/rel_repeat_axis_event_handler.hpp"

#include <math.h>
#include <sstream>

#include <uinpp/multi_device.hpp>

#include "evdev_helper.hpp"
#include "util/string.hpp"
#include "raise_exception.hpp"

RelRepeatAxisEventHandler*
RelRepeatAxisEventHandler::from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                       const std::string& str)
{
  // split string at ':'
  std::vector<std::string> args = string_split(str, ":");

  if (args.size() == 3)
  {
    return new RelRepeatAxisEventHandler(uinput, slot, extra_devices,
                                         str2rel_event(args[0]),
                                         str2int(args[1]),
                                         str2float(args[2]));
  }
  else
  {
    raise_exception(std::runtime_error, "must have three arguments");
  }
}

RelRepeatAxisEventHandler::RelRepeatAxisEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                                     const uinpp::Event& code, int value, float repeat) :
  m_code(code),
  m_value(value),
  m_repeat(repeat),
  m_stick_value(0),
  m_timer(0),
  m_rel_emitter()
{
  m_code.resolve_device_id(slot, extra_devices);
  m_rel_emitter = uinput.add_rel(m_code.get_device_id(), m_code.code);
}

void
RelRepeatAxisEventHandler::send(int value, int min, int max)
{
  if (value < 0)
  {
    m_stick_value = static_cast<float>(value) / static_cast<float>(-min);
  }
  else
  {
    m_stick_value = static_cast<float>(value) / static_cast<float>(max);
  }

  // reset timer when in center position
  if (value == 0)
  {
    m_timer = 0;
  }
}

void
RelRepeatAxisEventHandler::update(int msec_delta)
{
  // time ticks slower depending on how far the stick is moved
  m_timer += static_cast<float>(msec_delta) * fabsf(m_stick_value);

  while(m_timer > m_repeat)
  {
    if (m_stick_value < 0)
    {
      m_rel_emitter->send(-m_value);
    }
    else
    {
      m_rel_emitter->send(m_value);
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
