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

#ifndef HEADER_XBOXDRV_BUTTON_MAP_HPP
#define HEADER_XBOXDRV_BUTTON_MAP_HPP

#include <assert.h>

#include "button_event.hpp"
#include "xboxmsg.hpp"

class ButtonMap
{
private:
  ButtonEventPtr btn_map[XBOX_BTN_MAX][XBOX_BTN_MAX];
  
public:
  ButtonMap();

  void bind(XboxButton code, ButtonEventPtr event);
  void bind(XboxButton shift_code, XboxButton code, ButtonEventPtr event);

  ButtonEventPtr lookup(XboxButton code) const;
  ButtonEventPtr lookup(XboxButton shift_code, XboxButton code) const;

  void init(uInput& uinput, int slot, bool extra_devices) const;

  bool send(uInput& uinput, XboxButton code, bool value) const;
  bool send(uInput& uinput, XboxButton shift_code, XboxButton code, bool value) const;
  void update(uInput& uinput, int msec_delta);

  void clear();
};

#endif

/* EOF */
