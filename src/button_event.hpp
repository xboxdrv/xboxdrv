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

#ifndef HEADER_XBOXDRV_BUTTON_EVENT_HPP
#define HEADER_XBOXDRV_BUTTON_EVENT_HPP

#include <string>

#include "uinput_deviceid.hpp"

class uInput;

struct ButtonEvent
{
  static const int MAX_MODIFIER = 4;
  static ButtonEvent invalid() { return create(-1); }
  static ButtonEvent create(int type);
  static ButtonEvent create_btn(int code);
  static ButtonEvent create_btn();
  static ButtonEvent create_rel(int code);
  static ButtonEvent from_string(const std::string& str);

  /** EV_KEY, EV_ABS, EV_REL */
  int type;

  union {
    struct {
      UIEvent code;
      int  repeat;
      int  value;
    } rel;

    struct {
      UIEvent code;
      int value;
    } abs;

    struct {
      // Array is terminated by !is_valid()
      UIEvent codes[MAX_MODIFIER+1];
    } key;
  };

  void init(uInput& uinput);
  void send(uInput& uinput, bool value) const;

  bool is_valid() const;
};

#endif

/* EOF */
