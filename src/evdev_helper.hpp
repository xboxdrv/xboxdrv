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

#include <string>

#include "uinput_deviceid.hpp"

void str2event(const std::string& name, int& type, int& code);
int  get_event_type(const std::string& str);

int str2btn(const std::string& str);
int str2abs(const std::string& str);
int str2rel(const std::string& str);

std::string btn2str(int i);
std::string abs2str(int i);
std::string rel2str(int i);

UIEvent str2btn_event(const std::string& str);
UIEvent str2rel_event(const std::string& str);
UIEvent str2key_event(const std::string& str);

#endif

/* EOF */
