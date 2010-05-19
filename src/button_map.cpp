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

#include "button_map.hpp"

ButtonMap::ButtonMap()
{
  clear();
}

void
ButtonMap::bind(int code, const ButtonEvent& event)
{
  btn_map[XBOX_BTN_UNKNOWN][code] = event;
}

void
ButtonMap::bind(int shift_code, int code, const ButtonEvent& event)
{
  btn_map[shift_code][code] = event;
}

ButtonEvent
ButtonMap::lookup(int code) const
{
  return btn_map[XBOX_BTN_UNKNOWN][code];
}

ButtonEvent
ButtonMap::lookup_shift(int shift_code, int code) const
{
  return btn_map[shift_code][code];
}

void
ButtonMap::clear()
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_BTN_MAX; ++code)
    {
      btn_map[shift_code][code] = ButtonEvent::invalid();
    }
  }
}

/* EOF */
