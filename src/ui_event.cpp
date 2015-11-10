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

#include "ui_event.hpp"

#include <boost/lexical_cast.hpp>

#include "evdev_helper.hpp"
#include "helper.hpp"
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
UIEvent::create(uint16_t device_id, int type, int code)
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
UIEvent::from_char(char c)
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_AUTO;
  ev.m_device_id_resolved = false;
  ev.type = EV_KEY;

  // FIXME: not very complete, should use UIEventSequence and do a bit more magic
  switch(c)
  {
    case 'a': ev.code = str2key("XK_a"); break;
    case 'b': ev.code = str2key("XK_b"); break;
    case 'c': ev.code = str2key("XK_c"); break;
    case 'd': ev.code = str2key("XK_d"); break;
    case 'e': ev.code = str2key("XK_e"); break;
    case 'f': ev.code = str2key("XK_f"); break;
    case 'g': ev.code = str2key("XK_g"); break;
    case 'h': ev.code = str2key("XK_h"); break;
    case 'i': ev.code = str2key("XK_i"); break;
    case 'j': ev.code = str2key("XK_j"); break;
    case 'k': ev.code = str2key("XK_k"); break;
    case 'l': ev.code = str2key("XK_l"); break;
    case 'm': ev.code = str2key("XK_m"); break;
    case 'n': ev.code = str2key("XK_n"); break;
    case 'o': ev.code = str2key("XK_o"); break;
    case 'p': ev.code = str2key("XK_p"); break;
    case 'q': ev.code = str2key("XK_q"); break;
    case 'r': ev.code = str2key("XK_r"); break;
    case 's': ev.code = str2key("XK_s"); break;
    case 't': ev.code = str2key("XK_t"); break;
    case 'u': ev.code = str2key("XK_u"); break;
    case 'v': ev.code = str2key("XK_v"); break;
    case 'w': ev.code = str2key("XK_w"); break;
    case 'x': ev.code = str2key("XK_x"); break;
    case 'y': ev.code = str2key("XK_y"); break;
    case 'z': ev.code = str2key("XK_z"); break;
    case '0': ev.code = str2key("XK_0"); break;
    case '1': ev.code = str2key("XK_1"); break;
    case '2': ev.code = str2key("XK_2"); break;
    case '3': ev.code = str2key("XK_3"); break;
    case '4': ev.code = str2key("XK_4"); break;
    case '5': ev.code = str2key("XK_5"); break;
    case '6': ev.code = str2key("XK_6"); break;
    case '7': ev.code = str2key("XK_7"); break;
    case '8': ev.code = str2key("XK_8"); break;
    case '9': ev.code = str2key("XK_9"); break;

    case '.': ev.code = str2key("XK_period"); break;

    case '\n': ev.code = str2key("XK_enter"); break;

    default:  ev.code = str2key("XK_space"); break;
  }
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
  // BROKEN: must take all members into account
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
    m_slot_id = static_cast<uint16_t>(slot);
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

uint16_t str2deviceid(const std::string& device)
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
    return boost::lexical_cast<uint16_t>(device);
  }
}

uint16_t str2slotid(const std::string& slot)
{
  if (slot == "auto" || slot.empty())
  {
    return SLOTID_AUTO;
  }
  else
  {
    return boost::lexical_cast<uint16_t>(slot);
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

/* EOF */
