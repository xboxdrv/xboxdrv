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

#include "button_event_factory.hpp"

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

ButtonEventFactory::ButtonEventFactory(UInput& uinput, int slot, bool extra_devices) :
  m_uinput(uinput),
  m_slot(slot),
  m_extra_devices(extra_devices)
{
}

ButtonEventPtr
ButtonEventFactory::create(ButtonEventHandler* handler)
{
  return ButtonEventPtr(new ButtonEvent(handler));
}

ButtonEventPtr
ButtonEventFactory::create_abs(int code)
{
  return create(new AbsButtonEventHandler(code));
}

ButtonEventPtr 
ButtonEventFactory::create_key(int device_id, int code)
{
  return create(new KeyButtonEventHandler(device_id, code));
}

ButtonEventPtr
ButtonEventFactory::create_key(int code)
{
  return create(new KeyButtonEventHandler(DEVICEID_AUTO, code));
}

ButtonEventPtr
ButtonEventFactory::create_key()
{
  return create(new KeyButtonEventHandler);
}

ButtonEventPtr
ButtonEventFactory::create_rel(int code)
{
  return create(new RelButtonEventHandler(UIEvent::create(DEVICEID_AUTO, EV_REL, code)));
}

ButtonEventPtr
ButtonEventFactory::from_string(const std::string& str, const std::string& directory)
{
  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (token == "abs")
  {
    return create(AbsButtonEventHandler::from_string(rest));
  }
  else if (token == "rel")
  {
    return create(RelButtonEventHandler::from_string(rest));
  }
  else if (token == "key")
  {
    return create(KeyButtonEventHandler::from_string(rest));
  }
  else if (token == "cycle-key")
  {
    return create(CycleKeyButtonEventHandler::from_string(rest, true));
  }
  else if (token == "cycle-key-named")
  {
    return create(CycleKeyButtonEventHandler::from_string_named(rest, true));
  }
  else if (token == "sequence-key-named" || token == "seq-key-named")
  {
    return create(CycleKeyButtonEventHandler::from_string_named(rest, false));
  }
  else if (token == "cycle-key-ref" || token == "seq-key-ref" || token == "sequence-key-ref")
  {
    return create(CycleKeyButtonEventHandler::from_string_ref(rest));
  }
  else if (token == "exec")
  {
    return create(ExecButtonEventHandler::from_string(rest));
  }
  else if (token == "macro")
  {
    return create(MacroButtonEventHandler::from_string(path::join(directory, rest)));
  }
  else
  {
    // try to guess the type of event on the type of the first event code
    switch(get_event_type(token))
    {
      case EV_KEY: return create(KeyButtonEventHandler::from_string(str));
      case EV_REL: return create(RelButtonEventHandler::from_string(str));
      case EV_ABS: return create(AbsButtonEventHandler::from_string(str));
      case     -1: return ButtonEventPtr(); // void
      default: assert(!"unknown type");
    }
  }
}

/* EOF */
