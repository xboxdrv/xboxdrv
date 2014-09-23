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

#include <boost/tokenizer.hpp>

#include "raise_exception.hpp"
#include "ui_event.hpp"
#include "evdev_helper.hpp"
#include "uinput.hpp"

bool
UIEvent::is_mouse_button(int ev_code)
{
  return  (ev_code >= BTN_MOUSE && ev_code <= BTN_TASK);
}

bool
UIEvent::is_keyboard_button(int ev_code)
{
  return (ev_code < 256);
}

UIEvent
UIEvent::create(int device_id, int type, int code) 
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = device_id;
  ev.m_device_id_resolved = false;
  ev.type      = type;
  ev.code      = code;
  return ev;
}

UIEvent
UIEvent::from_string(const std::string& str)
{
  switch(get_event_type(str))
  {
    case EV_REL: return str2rel_event(str); break;
    case EV_ABS: return str2abs_event(str); break;
    case EV_KEY: return str2key_event(str); break;
    default: throw std::runtime_error("unknown event type");
  }
}

UIEvent
UIEvent::invalid()
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_INVALID;
  ev.m_device_id_resolved = false;
  ev.type      = -1;
  ev.code      = -1;
  return ev;    
}

bool
UIEvent::operator<(const UIEvent& rhs)  const
{
  if (m_device_id == rhs.m_device_id)
  {
    if (type == rhs.type)
    {
      return code < rhs.code;
    }
    else if (type > rhs.type)
    {
      return false;
    }
    else // if (type < rhs.type)
    {
      return true;
    }
  }
  else if (m_device_id > rhs.m_device_id)
  {
    return false;
  }
  else // if (device_id < rhs.device_id)
  {
    return true;
  }
}

void
UIEvent::resolve_device_id(int slot, bool extra_devices)
{
  assert(!m_device_id_resolved);

  if (m_slot_id == SLOTID_AUTO)
  {
    m_slot_id = slot;
  }

  if (m_device_id == DEVICEID_AUTO)
  {
    if (extra_devices)
    {
      switch(type)
      {
        case EV_KEY:
          if (is_mouse_button(code))
          {
            m_device_id = DEVICEID_MOUSE;
          }
          else if (is_keyboard_button(code))
          {
            m_device_id = DEVICEID_KEYBOARD;
          }
          else
          {
            m_device_id = DEVICEID_JOYSTICK;
          }
          break;

        case EV_REL:
          m_device_id = DEVICEID_MOUSE;
          break;

        case EV_ABS:
          m_device_id = DEVICEID_JOYSTICK;
          break;
      }
    }
    else
    {
      m_device_id = DEVICEID_JOYSTICK;
    }
  }

  m_device_id_resolved = true;
}

uint32_t
UIEvent::get_device_id() const
{
  assert(m_device_id_resolved);

  return UInput::create_device_id(m_slot_id, m_device_id);
}

int str2deviceid(const std::string& device)
{
  if (device == "auto" || device.empty())
  {
    return DEVICEID_AUTO;
  }
  else if (device == "mouse")
  {
    return DEVICEID_MOUSE;
  }
  else if (device == "keyboard" || device == "key")
  {
    return DEVICEID_KEYBOARD;
  }
  else if (device == "joystick" || device == "joy")
  {
    return DEVICEID_JOYSTICK;
  }
  else
  {
    return boost::lexical_cast<int>(device);
  }
}

int str2slotid(const std::string& slot)
{
  if (slot == "auto" || slot.empty())
  {
    return SLOTID_AUTO;
  }
  else
  {
    return boost::lexical_cast<int>(slot);
  }
}

void split_event_name(const std::string& str, std::string* event_str, int* slot_id, int* device_id)
{
  std::string::size_type p = str.find('@');
  if (p == std::string::npos)
  {
    *event_str = str;
    *slot_id   = SLOTID_AUTO;
    *device_id = DEVICEID_AUTO;
  }
  else
  {
    *event_str = str.substr(0, p);
    std::string device = str.substr(p+1);
    
    p = device.find(".");
    
    if (p == std::string::npos)
    {
      *slot_id   = SLOTID_AUTO;
      *device_id = str2deviceid(device);
    }
    else
    {
      *slot_id   = str2slotid(device.substr(p+1));
      *device_id = str2deviceid(device.substr(0, p));
    }
  }
}

UIAction::AxisAction UIAction::string2axis_action(const std::string &str) {
    AxisAction a;
    a.rezero = FALSE;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer ev_tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
    int i = 0;
    for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, i++) {
        if (i == 0) {
            a.axis = string2axis(*m);
        } else if (i == 1) {
            a.set_value = boost::lexical_cast<int>(*m);
        } else if (i == 2) {
            a.zero_value = boost::lexical_cast<int>(*m);
            a.rezero = TRUE;
        } else {
            raise_exception(std::runtime_error, "Can't parse string: too many items: \"" + str + "\"");
        }
    }
    if (i < 2) {
        raise_exception(std::runtime_error, "Not enough arguments: \"" + str + "\"");
    }
    return a;
}

UIAction::UIAction(const ButtonList buttons, const AxesList axes) :
    btns(buttons),
    axes(axes)
{
}

UIActionPtr UIAction::from_string(const std::string &value) {
    ButtonList buttons;
    AxesList axs;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer ev_tokens(value, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
    for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m)
    {
        std::string event_name;
        std::string token = *m;
        switch(get_event_type(token))
        {
          case EV_KEY: {
            event_name = token.substr(token.find('_') + 1, std::string::npos);
            buttons.push_back(string2btn(event_name));
            break;
          }
          case EV_REL: {
            raise_exception(std::runtime_error, "Relative events not supported.");
            break;
          }
          case EV_ABS: {
            event_name = token.substr(token.find('_') + 1, std::string::npos);
            axs.push_back(string2axis_action(event_name));
            break;
          }
          default: {
            raise_exception(std::runtime_error, "Unsupported event \"" + token + "\".");
          }
        }
    }
    return UIActionPtr(new UIAction(buttons, axs));
}

void UIAction::parse(XboxGenericMsg &msg, const struct input_event& ev) {
    for (ButtonList::iterator it = btns.begin(); it != btns.end(); ++it) {
        set_button(msg, *it, ev.value);
    }
    for (AxesList::iterator it = axes.begin(); it != axes.end(); ++it) {
        AxisAction a = *it;
        if (ev.value || a.rezero) {
            int value;
            if (ev.value) {
                value = a.set_value;
            } else {
                value = a.zero_value;
            }
            std::cout << ev.value << " " << a.rezero << " " << a.set_value << " " << a.zero_value << " = " << value << std::endl;
            set_axis(msg, a.axis, value);
        }
    }
}

/* EOF */
