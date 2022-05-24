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

#include "abs_axis_event_handler.hpp"

#include <uinpp/multi_device.hpp>
#include <strut/split.hpp>

#include "evdev_helper.hpp"
#include "util/string.hpp"
#include "raise_exception.hpp"

AbsAxisEventHandler*
AbsAxisEventHandler::from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                 const std::string& str)
{
  auto tokens = strut::split(str, ':');

  int min = -1;
  int max = -1;
  int fuzz = 0;
  int flat = 0;

  int j = 0;
  uinpp::Event code = uinpp::Event::invalid();
  for(auto i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        code = str2abs_event(*i);
        break;

      case 1:
        min = str2int(*i);
        break;

      case 2:
        max = str2int(*i);
        break;

      case 3:
        fuzz = str2int(*i);
        break;

      case 4:
        flat = str2int(*i);
        break;

      default:
        raise_exception(std::runtime_error, "to many arguments: " + str);
    }
  }

  if (j == 0)
  {
    raise_exception(std::runtime_error, "at least one argument required: " + str);
  }
  else
  {
    return new AbsAxisEventHandler(uinput, slot, extra_devices,
                                   code, min, max, fuzz, flat);
  }
}

AbsAxisEventHandler::AbsAxisEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                                         const uinpp::Event& code, int min, int max, int fuzz, int flat) :
  m_code(code),
  m_min(min),
  m_max(max),
  m_fuzz(fuzz),
  m_flat(flat),
  m_abs_emitter()
{
  m_code.resolve_device_id(slot, extra_devices);
  m_abs_emitter = uinput.add_abs(m_code.get_device_id(), m_code.code,
                                 m_min, m_max, m_fuzz, m_flat);
}

void
AbsAxisEventHandler::send(int value, int min, int max)
{
  m_abs_emitter->send(value);
}

void
AbsAxisEventHandler::update(int msec_delta)
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

/* EOF */
