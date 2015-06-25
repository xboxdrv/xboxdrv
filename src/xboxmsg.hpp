/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
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
  GAMEPAD_XBOXONE_WIRELESS,
  GAMEPAD_FIRESTORM,
  GAMEPAD_FIRESTORM_VSB,
  GAMEPAD_SAITEK_P2500,
  GAMEPAD_PLAYSTATION3_USB,
  GAMEPAD_PLAYSTATION3_BLUETOOTH,
  GAMEPAD_LOGITECH_F310,
  GAMEPAD_GENERIC_USB,
  GAMEPAD_WIIMOTE,
  GAMEPAD_HAMA_CRUX
};

enum XboxMsgType {
  XBOX_MSG_XBOX,
  XBOX_MSG_XBOX360,
  XBOX_MSG_PS3USB
};

std::ostream& operator<<(std::ostream& out, const GamepadType& type);

std::string gamepadtype_to_string(const GamepadType& type);
std::string gamepadtype_to_macro_string(const GamepadType& type);

#endif

/* EOF */
