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

#include "button_map.hpp"

ButtonMap::ButtonMap()
{
  clear();
}

void
ButtonMap::bind(XboxButton code, ButtonEventPtr event)
{
  btn_map[XBOX_BTN_UNKNOWN][code] = event;
}

void
ButtonMap::bind(XboxButton shift_code, XboxButton code, ButtonEventPtr event)
{
  btn_map[shift_code][code] = event;
}

ButtonEventPtr
ButtonMap::lookup(XboxButton code) const
{
  return btn_map[XBOX_BTN_UNKNOWN][code];
}

ButtonEventPtr
ButtonMap::lookup(XboxButton shift_code, XboxButton code) const
{
  return btn_map[shift_code][code];
}

bool
ButtonMap::send(UInput& uinput, XboxButton code, bool value) const
{
  return send(uinput, XBOX_BTN_UNKNOWN, code, value);
}

bool
ButtonMap::send(UInput& uinput, XboxButton shift_code, XboxButton code, bool value) const
{
  const ButtonEventPtr& event = lookup(shift_code, code);
  if (event)
  {
    event->send(uinput, value);
    return true;
  }
  else
  {
    return false;
  }
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

void
ButtonMap::init(UInput& uinput, int slot, bool extra_devices) const
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_BTN_MAX; ++code)
    {
      if (btn_map[shift_code][code])
      {
        btn_map[shift_code][code]->init(uinput, slot, extra_devices);
      }
    }
  }
}

void
ButtonMap::update(UInput& uinput, int msec_delta)
{
  for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
  {
    for(int code = 0; code < XBOX_BTN_MAX; ++code)
    {
      if (btn_map[shift_code][code])
      {
        btn_map[shift_code][code]->update(uinput, msec_delta);
      }
    }
  }
}

/* EOF */
