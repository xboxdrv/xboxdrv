/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_EVDEV_HELPER_HPP
#define HEADER_EVDEV_HELPER_HPP

#include <X11/Xlib.h>

#include "enum_box.hpp"
#include "ui_event.hpp"

void str2event(const std::string& name, int& type, int& code);
int  get_event_type(const std::string& str);

std::string key2str(int v);
std::string abs2str(int v);
std::string rel2str(int v);

int str2key(const std::string& str);
int str2abs(const std::string& str);
int str2rel(const std::string& str);

UIEvent str2key_event(const std::string& str);
UIEvent str2rel_event(const std::string& str);
UIEvent str2abs_event(const std::string& str);

class EvDevRelEnum : public EnumBox<int>
{
public:
  EvDevRelEnum();
};

class EvDevAbsEnum : public EnumBox<int>
{
public:
  EvDevAbsEnum();
};

class EvDevKeyEnum : public EnumBox<int>
{
public:
  EvDevKeyEnum();
};

class X11KeysymEnum : public EnumBox<int>
{
public:
  X11KeysymEnum();

private:
  void process_keymap(Display* dpy);
};

extern EvDevRelEnum  evdev_rel_names;
extern EvDevKeyEnum  evdev_key_names;
extern EvDevAbsEnum  evdev_abs_names;
const X11KeysymEnum& get_x11keysym_names();

#endif

/* EOF */
