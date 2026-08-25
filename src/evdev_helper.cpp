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

#include <format>
#include <linux/input.h>
#include <libevdev/libevdev.h>

#include <logmich/log.hpp>
#include <uinpp/parse.hpp>

#include "raise_exception.hpp"
#include "util/string.hpp"

namespace xboxdrv {

namespace {

int code_from_name(unsigned int type, std::string const& name)
{
  int code = libevdev_event_code_from_name(type, name.c_str());
  if (code < 0)
  {
    raise_exception(std::runtime_error,
                    "unknown " << libevdev_event_type_get_name(type)
                    << " name: '" << name << "'");
  }
  return code;
}

std::string name_from_code(unsigned int type, int code, char const* hash_prefix)
{
  char const* name = libevdev_event_code_get_name(type, code);
  if (name)
  {
    return name;
  }
  return std::format("{}#{}", hash_prefix, code);
}

std::vector<std::string> list_names(unsigned int type, int max_code)
{
  std::vector<std::string> names;
  for (int code = 0; code <= max_code; ++code)
  {
    if (char const* name = libevdev_event_code_get_name(type, code))
    {
      names.emplace_back(name);
    }
  }
  return names;
}

} // namespace

X11KeysymEnum const& get_x11keysym_names()
{
  static X11KeysymEnum x11keysym_names;
  return x11keysym_names;
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

      char const* keysym_str = XKeysymToString(keysym);
      if (!keysym_str)
      {
        log_warn("couldn't convert keysym {} to string", keysym);
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

int xkeysym2keycode(std::string const& name)
{
  return get_x11keysym_names()[name];
}

void str2event(std::string const& name, int& type, int& code)
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

int get_event_type(std::string const& name)
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

int str2abs(std::string const& name)
{
  if (name.compare(0, 5, "ABS_#") == 0)
  {
    return str2int(name.substr(5));
  }
  return code_from_name(EV_ABS, name);
}

int str2key(std::string const& name)
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
    return code_from_name(EV_KEY, name);
  }
  else
  {
    throw std::runtime_error("str2key: couldn't convert string: '" + name + "'");
  }
}

int str2rel(std::string const& name)
{
  if (name.compare(0, 5, "REL_#") == 0)
  {
    return str2int(name.substr(5));
  }
  return code_from_name(EV_REL, name);
}

uinpp::Event str2key_event(std::string const& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  uinpp::split_event_name(str, &rest, &slot_id, &device_id);
  return uinpp::Event::create(static_cast<uint16_t>(device_id), EV_KEY, str2key(rest));
}

uinpp::Event str2rel_event(std::string const& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  uinpp::split_event_name(str, &rest, &slot_id, &device_id);
  return uinpp::Event::create(static_cast<uint16_t>(device_id), EV_REL, str2rel(rest));
}

uinpp::Event str2abs_event(std::string const& str)
{
  int slot_id;
  int device_id;
  std::string rest;
  uinpp::split_event_name(str, &rest, &slot_id, &device_id);
  return uinpp::Event::create(static_cast<uint16_t>(device_id), EV_ABS, str2abs(rest));
}

std::string key2str(int v)
{
  return name_from_code(EV_KEY, v, "KEY_");
}

std::string abs2str(int v)
{
  return name_from_code(EV_ABS, v, "ABS_");
}

std::string rel2str(int v)
{
  return name_from_code(EV_REL, v, "REL_");
}

std::vector<std::string> list_evdev_key_names()
{
  return list_names(EV_KEY, KEY_MAX);
}

std::vector<std::string> list_evdev_abs_names()
{
  return list_names(EV_ABS, ABS_MAX);
}

std::vector<std::string> list_evdev_rel_names()
{
  return list_names(EV_REL, REL_MAX);
}

} // namespace xboxdrv

/* EOF */
