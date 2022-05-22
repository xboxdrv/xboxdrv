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

#include "buttonevent/rel_button_event_handler.hpp"

#include <uinpp/uinput.hpp>

#include "evdev_helper.hpp"
#include "util/string.hpp"

RelButtonEventHandler*
RelButtonEventHandler::from_string(uinpp::UInput& uinput, int slot, bool extra_devices,
                                   const std::string& str)
{
  std::unique_ptr<RelButtonEventHandler> ev;

  int idx = 0;
  auto tokens = string_split(str, ":");

  for(auto i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0:
        ev.reset(new RelButtonEventHandler(uinput, slot, extra_devices,
                                           str2rel_event(*i)));
        break;

      case 1:
        ev->m_value  = str2int(*i);
        break;

      case 2:
        ev->m_repeat = str2int(*i);
        break;
    }
  }

  return ev.release();
}

RelButtonEventHandler::RelButtonEventHandler(uinpp::UInput& uinput, int slot, bool extra_devices,
                                             const uinpp::UIEvent& code) :
  m_code(code),
  m_value(3),
  m_repeat(100),
  m_rel_emitter()
{
  m_code.resolve_device_id(slot, extra_devices);
  m_rel_emitter = uinput.add_rel(m_code.get_device_id(), m_code.code);
}

void
RelButtonEventHandler::send(bool value)
{
  if (m_repeat == -1)
  {
    if (value)
    {
      m_rel_emitter->send(m_value);
    }
  }
  else
  {
    if (value)
    {
      //BROKEN:uinput.send_rel_repetitive(m_code, static_cast<float>(m_value), m_repeat);
    }
    else
    {
      //BROKEN:uinput.send_rel_repetitive(m_code, static_cast<float>(m_value), -1);
    }
  }
}

std::string
RelButtonEventHandler::str() const
{
  std::ostringstream out;
  out << "rel:" << m_code.get_device_id() << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}

/* EOF */
