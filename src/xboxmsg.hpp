/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
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

#ifndef HEADER_XBOXMSG_HPP
#define HEADER_XBOXMSG_HPP

#include <iosfwd>

enum GamepadType {
  GAMEPAD_UNKNOWN,
  GAMEPAD_XBOX,
  GAMEPAD_XBOX_MAT,
  GAMEPAD_XBOX360,
  GAMEPAD_XBOX360_WIRELESS,
  GAMEPAD_XBOX360_PLAY_N_CHARGE,
  GAMEPAD_XBOX360_GUITAR,
  GAMEPAD_FIRESTORM,
  GAMEPAD_FIRESTORM_VSB,
  GAMEPAD_SAITEK_P2500,
  GAMEPAD_PLAYSTATION3_USB,
  GAMEPAD_LOGITECH_F310,
  GAMEPAD_GENERIC_USB,
  GAMEPAD_WIIMOTE
};

enum XboxMsgType {
  XBOX_MSG_XBOX,
  XBOX_MSG_XBOX360,
  XBOX_MSG_PS3USB
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

  XBOX_BTN_MAX
};

enum XboxAxis {
  XBOX_AXIS_UNKNOWN,

  XBOX_AXIS_X1,
  XBOX_AXIS_Y1,

  XBOX_AXIS_X2,
  XBOX_AXIS_Y2,

  XBOX_AXIS_LT,
  XBOX_AXIS_RT,

  XBOX_AXIS_DPAD_X,
  XBOX_AXIS_DPAD_Y,

  XBOX_AXIS_TRIGGER,

  // Xbox1 analog button
  XBOX_AXIS_A,
  XBOX_AXIS_B,
  XBOX_AXIS_X,
  XBOX_AXIS_Y,
  XBOX_AXIS_BLACK,
  XBOX_AXIS_WHITE,

  XBOX_AXIS_MAX
};

std::ostream& operator<<(std::ostream& out, const GamepadType& type);

XboxButton string2btn(const std::string& str_);
XboxAxis   string2axis(const std::string& str_);
std::string btn2string(XboxButton btn);
std::string axis2string(XboxAxis axis);

std::string gamepadtype_to_string(const GamepadType& type);
std::string gamepadtype_to_macro_string(const GamepadType& type);

#endif

/* EOF */
