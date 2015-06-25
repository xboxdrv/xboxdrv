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

#include "axisevent/rel_axis_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <math.h>

#include "evdev_helper.hpp"
#include "uinput.hpp"

RelAxisEventHandler*
RelAxisEventHandler::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  std::auto_ptr<RelAxisEventHandler> ev(new RelAxisEventHandler);

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0:
        ev->m_code = str2rel_event(*i);
        break;

      case 1:
        ev->m_value = boost::lexical_cast<float>(*i);
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

  return ev.release();
}

RelAxisEventHandler::RelAxisEventHandler() :
  m_code(UIEvent::invalid()),
  m_value(5),
  m_repeat(10),
  m_stick_value(0.0f),
  m_rest_value(0.0f),
  m_rel_emitter()
{
}

RelAxisEventHandler::RelAxisEventHandler(int device_id, int code, int repeat, float value) :
  m_code(UIEvent::create(device_id, EV_REL, code)),
  m_value(value),
  m_repeat(repeat),
  m_stick_value(0.0f),
  m_rest_value(0.0f),
  m_rel_emitter()
{
}

void
RelAxisEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  m_code.resolve_device_id(slot, extra_devices);
  m_rel_emitter = uinput.add_rel(m_code.get_device_id(), m_code.code);
}

void
RelAxisEventHandler::send(UInput& uinput, int value)
{
  if (value < 0)
    m_stick_value = value / static_cast<float>(-m_min);
  else
    m_stick_value = value / static_cast<float>(m_max);

  if (m_repeat != -1)
  {
    // regular old style sending of REL events
    float v = m_value * m_stick_value;

    if (v == 0)
      uinput.send_rel_repetitive(m_code, v, -1);
    else
      uinput.send_rel_repetitive(m_code, v, m_repeat);
  }
}

void
RelAxisEventHandler::update(UInput& uinput, int msec_delta)
{
  if (m_repeat == -1 && m_stick_value != 0.0f)
  {
    // new and improved REL style event sending

    float rel_value = m_stick_value * m_value * static_cast<float>(msec_delta) / 1000.0f;

    // keep track of the rest that we lose when converting to integer
    rel_value += m_rest_value;
    m_rest_value = rel_value - truncf(rel_value);

    m_rel_emitter->send(static_cast<int>(rel_value));
  }
}

std::string
RelAxisEventHandler::str() const
{
  std::ostringstream out;
  out << m_code.get_device_id() << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}

/* EOF */
