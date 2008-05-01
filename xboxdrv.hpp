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

enum GamepadType {
  GAMEPAD_UNKNOWN,
  GAMEPAD_XBOX,
  GAMEPAD_XBOX_MAT,
  GAMEPAD_XBOX360,
  GAMEPAD_XBOX360_WIRELESS,
  GAMEPAD_XBOX360_GUITAR
};

struct XPadDevice {
  GamepadType type;
  uint16_t    idVendor;
  uint16_t    idProduct;
  const char* name;
};

#endif

/* EOF */
