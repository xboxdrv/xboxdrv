/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "button_event.hpp"

#include <boost/tokenizer.hpp>
#include <errno.h>
#include <fstream>

#include "evdev_helper.hpp"
#include "log.hpp"
#include "path.hpp"
#include "uinput.hpp"

#include "buttonevent/abs_button_event_handler.hpp"
#include "buttonevent/cycle_key_button_event_handler.hpp"
#include "buttonevent/exec_button_event_handler.hpp"
#include "buttonevent/key_button_event_handler.hpp"
#include "buttonevent/macro_button_event_handler.hpp"
#include "buttonevent/rel_button_event_handler.hpp"

ButtonEventPtr
ButtonEvent::invalid()
{
  return ButtonEventPtr();
}

ButtonEventPtr
ButtonEvent::create(ButtonEventHandler* handler)
{
  return ButtonEventPtr(new ButtonEvent(handler));
}

ButtonEventPtr
ButtonEvent::create_abs(int code)
{
  return ButtonEvent::create(new AbsButtonEventHandler(code));
}

ButtonEventPtr
ButtonEvent::create_key(int device_id, int code)
{
  return ButtonEvent::create(new KeyButtonEventHandler(device_id, code));
}

ButtonEventPtr
ButtonEvent::create_key(int code)
{
  return ButtonEvent::create(new KeyButtonEventHandler(DEVICEID_AUTO, code));
}

ButtonEventPtr
ButtonEvent::create_key()
{
  return ButtonEvent::create(new KeyButtonEventHandler);
}

ButtonEventPtr
ButtonEvent::create_rel(int code)
{
  return ButtonEvent::create(new RelButtonEventHandler(UIEvent::create(DEVICEID_AUTO, EV_REL, code)));
}

ButtonEventPtr
ButtonEvent::from_string(const std::string& str, const std::string& directory)
{
  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos)
    rest = str.substr(p+1);

  if (token == "abs")
  {
    return ButtonEvent::create(AbsButtonEventHandler::from_string(rest));
  }
  else if (token == "rel")
  {
    return ButtonEvent::create(RelButtonEventHandler::from_string(rest));
  }
  else if (token == "key")
  {
    return ButtonEvent::create(KeyButtonEventHandler::from_string(rest));
  }
  else if (token == "cycle-key")
  {
    return ButtonEvent::create(CycleKeyButtonEventHandler::from_string(rest, true));
  }
  else if (token == "cycle-key-named")
  {
    return ButtonEvent::create(CycleKeyButtonEventHandler::from_string_named(rest, true));
  }
  else if (token == "sequence-key-named" || token == "seq-key-named")
  {
    return ButtonEvent::create(CycleKeyButtonEventHandler::from_string_named(rest, false));
  }
  else if (token == "cycle-key-ref" || token == "seq-key-ref" || token == "sequence-key-ref")
  {
    return ButtonEvent::create(CycleKeyButtonEventHandler::from_string_ref(rest));
  }
  else if (token == "exec")
  {
    return ButtonEvent::create(ExecButtonEventHandler::from_string(rest));
  }
  else if (token == "macro")
  {
    return ButtonEvent::create(MacroButtonEventHandler::from_string(path::join(directory, rest)));
  }
  else
  {
    // try to guess the type of event on the type of the first event code
    switch(get_event_type(token))
    {
      case EV_KEY: return ButtonEvent::create(KeyButtonEventHandler::from_string(str));
      case EV_REL: return ButtonEvent::create(RelButtonEventHandler::from_string(str));
      case EV_ABS: return ButtonEvent::create(AbsButtonEventHandler::from_string(str));
      case     -1: return ButtonEvent::invalid(); // void
      default: assert(!"unknown type");
    }
  }
}

ButtonEvent::ButtonEvent(ButtonEventHandler* handler) :
  m_last_send_state(false),
  m_last_raw_state(false),
  m_handler(handler),
  m_filters()
{
}

void
ButtonEvent::add_filters(const std::vector<ButtonFilterPtr>& filters)
{
  std::copy(filters.begin(), filters.end(), std::back_inserter(m_filters));
}

void
ButtonEvent::add_filter(ButtonFilterPtr filter)
{
  m_filters.push_back(filter);
}

void
ButtonEvent::init(UInput& uinput, int slot, bool extra_devices)
{
  return m_handler->init(uinput, slot, extra_devices);
}

void
ButtonEvent::send(UInput& uinput, bool raw_state)
{
  m_last_raw_state = raw_state;
  bool filtered_state = raw_state;

  // filter values
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    filtered_state = (*i)->filter(filtered_state);
  }

  if (m_last_send_state == filtered_state)
  {
    // button state has not changed, so do not send events
  }
  else
  {
    m_last_send_state = filtered_state;
    m_handler->send(uinput, m_last_send_state);
  }
}

void
ButtonEvent::update(UInput& uinput, int msec_delta)
{
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  m_handler->update(uinput, msec_delta);

  send(uinput, m_last_raw_state);
}

std::string
ButtonEvent::str() const
{
  return m_handler->str();
}

/* EOF */
