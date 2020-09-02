/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include "evdev_helper.hpp"

#include <linux/input.h>

#include "util/math.hpp"
#include "util/string.hpp"
#include "log.hpp"

EvDevRelEnum evdev_rel_names;
EvDevKeyEnum evdev_key_names;
EvDevAbsEnum evdev_abs_names;

const X11KeysymEnum& get_x11keysym_names()
{
  static X11KeysymEnum x11keysym_names;
  return x11keysym_names;
}

EvDevRelEnum::EvDevRelEnum() :
  EnumBox<int>("EV_REL")
{
#  include "rel_list.x"
}

EvDevAbsEnum::EvDevAbsEnum() :
    EnumBox<int>("EV_ABS")
{
#  include "abs_list.x"
}


EvDevKeyEnum::EvDevKeyEnum() :
  EnumBox<int>("EV_KEY")
{
#  include "key_list.x"
}

X11KeysymEnum::X11KeysymEnum() :
  EnumBox<int>("X11Keysym")
{
  Display* dpy = XOpenDisplay(NULL);
  if (!dpy)
  {
    log_error("unable to open X11 display, X11 keynames will not be available");
  }
  else
  {
    process_keymap(dpy);
    XCloseDisplay(dpy);
  }
}

void
X11KeysymEnum::process_keymap(Display* dpy)
{
  int min_keycode, max_keycode;
  XDisplayKeycodes(dpy, &min_keycode, &max_keycode);

  int num_keycodes = max_keycode - min_keycode + 1;
  int keysyms_per_keycode;
  KeySym* keymap = XGetKeyboardMapping(dpy, static_cast<KeyCode>(min_keycode),
                                       num_keycodes,
                                       &keysyms_per_keycode);

  for(int i = 0; i < num_keycodes; ++i)
  {
    if (keymap[i*keysyms_per_keycode] != NoSymbol)
    {
      KeySym keysym = keymap[i*keysyms_per_keycode];

      // FIXME: Duplicate entries confuse the conversion
      // std::map<KeySym, int>::iterator it = mapping.find(keysym);
      // if (it != mapping.end())
      //   std::cout << "Duplicate keycode: " << i << std::endl;

      const char* keysym_str = XKeysymToString(keysym);
      if (!keysym_str)
      {
        log_warn("couldn't convert keysym " << keysym << " to string");
      }
      else
      {
        std::ostringstream str;
        str << "XK_" << keysym_str;
        add(i, str.str());
      }
    }
  }

  XFree(keymap);
}

int xkeysym2keycode(const std::string& name)
{
  return get_x11keysym_names()[name];
}

void str2event(const std::string& name, int& type, int& code)
{
  if (name == "void" || name == "none")
  {
    type = -1;
    code = -1;
  }
  else if (name.compare(0, 3, "REL") == 0)
  {
    type = EV_REL;
    code = str2rel(name);
  }
  else if (name.compare(0, 3, "ABS") == 0)
  {
    type = EV_ABS;
    code = str2abs(name);
  }
  else if (name.compare(0, 2, "XK") == 0)
  {
    type = EV_KEY;
    code = xkeysym2keycode(name);
  }
  else if (name.compare(0, 2, "JS") == 0)
  {
    type = EV_KEY;
    code = BTN_JOYSTICK + str2int(name.substr(3));
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0)
  {
    type = EV_KEY;
    code = str2key(name);
  }
  else
  {
    throw std::runtime_error("str2event(): unknown event type prefix: " + name);
  }
}

int get_event_type(const std::string& name)
{
  if (name == "void" || name == "none")
  {
    return -1;
  }
  else if (name.compare(0, 3, "REL") == 0)
  {
    return EV_REL;
  }
  else if (name.compare(0, 3, "ABS") == 0)
  {
    return EV_ABS;
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0 ||
           name.compare(0, 2, "JS")  == 0 ||
           name.compare(0, 2, "XK")  == 0)
  {
    return EV_KEY;
  }
  else
  {
    throw std::runtime_error("get_event_type(): unknown event type prefix: " + name);
  }
}

int str2abs(const std::string& name)
{
  if (name.compare(0, 5, "ABS_#") == 0)
  {
    return str2int(name.substr(5));
  }
  else
  {
    return evdev_abs_names[name];
  }
}

int str2key(const std::string& name)
{
  if (name.compare(0, 2, "XK") == 0)
  {
    return xkeysym2keycode(name);
  }
  else if (name.compare(0, 2, "JS") == 0)
  {
    return BTN_JOYSTICK + str2int(name.substr(3));
  }
  else if (name.compare(0, 5, "KEY_#") == 0)
  {
    return str2int(name.substr(5));
  }
  else if (name.compare(0, 3, "KEY") == 0 ||
           name.compare(0, 3, "BTN") == 0)
  {
    return evdev_key_names[name];
  }
  else
  {
    throw std::runtime_error("str2key: couldn't convert string: '" + name + "'");
  }
}

int str2rel(const std::string& name)
{
  if (name.compare(0, 5, "REL_#") == 0)
  {
    return str2int(name.substr(5));
  }
  else
  {
    return evdev_rel_names[name];
  }
}

UIEvent str2key_event(const std::string& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &slot_id, &device_id);
  return UIEvent::create(static_cast<uint16_t>(device_id), EV_KEY, str2key(rest));
}

UIEvent str2rel_event(const std::string& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &slot_id, &device_id);
  return UIEvent::create(static_cast<uint16_t>(device_id), EV_REL, str2rel(rest));
}

UIEvent str2abs_event(const std::string& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  split_event_name(str, &rest, &slot_id, &device_id);
  return UIEvent::create(static_cast<uint16_t>(device_id), EV_ABS, str2abs(rest));
}

std::string key2str(int v)
{
  try
  {
    return evdev_key_names[v];
  }
  catch(const std::exception& err)
  {
    std::ostringstream str;
    str << "KEY_#" << v;
    return str.str();
  }
}

std::string abs2str(int v)
{
  try
  {
    return evdev_abs_names[v];
  }
  catch(const std::exception& err)
  {
    std::ostringstream str;
    str << "ABS_#" << v;
    return str.str();
  }
}

std::string rel2str(int v)
{
  try
  {
    return evdev_rel_names[v];
  }
  catch(const std::exception& err)
  {
    std::ostringstream str;
    str << "REL_#" << v;
    return str.str();
  }
}

/* EOF */
