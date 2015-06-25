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

#include <boost/tokenizer.hpp>

#include "evdev_helper.hpp"
#include "uinput.hpp"

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
  m_flat(0),
  m_abs_emitter()
{
}

AbsAxisEventHandler::AbsAxisEventHandler(const UIEvent& code, int min, int max, int fuzz, int flat) :
  m_code(code),
  m_fuzz(fuzz),
  m_flat(flat),
  m_abs_emitter()
{
  set_axis_range(min, max);
}

void
AbsAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  assert(!m_abs_emitter);

  m_code.resolve_device_id(slot, extra_devices);
  m_abs_emitter = uinput.add_abs(m_code.get_device_id(), m_code.code,
                                 m_min, m_max, m_fuzz, m_flat);
}

void
AbsAxisEventHandler::send(UInput& uinput, int value)
{
  m_abs_emitter->send(value);
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

/* EOF */
