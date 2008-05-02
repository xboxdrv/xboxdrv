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

#ifndef HEADER_XBOX360_HPP
#define HEADER_XBOX360_HPP

#include "xboxmsg.hpp"

struct XPadDevice {
  GamepadType type;
  uint16_t    idVendor;
  uint16_t    idProduct;
  const char* name;
};

enum XboxButton {
  XBOX_BTN_UNKNOWN,
  XBOX_BTN_START,
  XBOX_BTN_GUIDE,
  XBOX_BTN_BACK,

  XBOX_BTN_A,
  XBOX_BTN_B,
  XBOX_BTN_X,
  XBOX_BTN_Y,

  XBOX_BTN_WHITE,
  XBOX_BTN_BLACK,

  XBOX_BTN_LB,
  XBOX_BTN_RB,

  XBOX_BTN_LT,
  XBOX_BTN_RT,

  XBOX_BTN_THUMB_L,
  XBOX_BTN_THUMB_R,

  XBOX_DPAD_UP,
  XBOX_DPAD_DOWN,
  XBOX_DPAD_LEFT,
  XBOX_DPAD_RIGHT,
};

enum XboxAxis {
  XBOX_AXIS_UNKNOWN,
  XBOX_AXIS_X1,
  XBOX_AXIS_Y1,
  XBOX_AXIS_X2,
  XBOX_AXIS_Y2,
  XBOX_AXIS_LT,
  XBOX_AXIS_RT,
};

struct ButtonMapping {
  XboxButton lhs;
  XboxButton rhs;
};

struct AxisMapping {
  XboxAxis lhs;
  XboxAxis rhs;
  bool     invert;
};

#endif

/* EOF */
