/* 
**  XBox360 USB Gamepad Userspace Driver
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

enum XBox360Buttons {
  XBOX360_DPAD_UP    = (1<< 0),
  XBOX360_DPAD_DOWN  = (1<< 1),
  XBOX360_DPAD_LEFT  = (1<< 2),
  XBOX360_DPAD_RIGHT = (1<< 3),
  XBOX360_START      = (1<< 4),
  XBOX360_SELECT     = (1<< 5),
  XBOX360_THUMB_L    = (1<< 6),
  XBOX360_THUMB_R    = (1<< 7),
  XBOX360_LB         = (1<< 8),
  XBOX360_RB         = (1<< 9),
  XBOX360_MODE       = (1<<10),
  // unused          = (1<<11),
  XBOX360_A          = (1<<12),
  XBOX360_B          = (1<<13),
  XBOX360_X          = (1<<14),
  XBOX360_Y          = (1<<15),
};

struct XBox360Msg
{
  // -------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int select      :1;

  unsigned int thumb_l     :1;
  unsigned int thumb_r     :1;

  // data[3] ------------------
  unsigned int lb          :1;
  unsigned int rb          :1;
  unsigned int mode        :1;
  unsigned int dummy1      :1;

  unsigned int a           :1;
  unsigned int b           :1;
  unsigned int x           :1;
  unsigned int y           :1;

  // data[4] ------------------
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  int x1                   :16;
  int y1                   :16;

  // data[10] -----------------
  int x2                   :16;
  int y2                   :16;

  // data[14]; ----------------
  unsigned int dummy2      :32;
  unsigned int dummy3      :16;
} __attribute__((__packed__));


struct XBox360GuitarMsg
{
  // -------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1; // also strum-up
  unsigned int dpad_down   :1; // also strum-down
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int select      :1;

  unsigned int thumb_l     :1; // unused
  unsigned int thumb_r     :1; // unused

  // data[3] ------------------
  unsigned int orange      :1; // 5
  unsigned int rb          :1; // unused
  unsigned int mode        :1; 
  unsigned int dummy1      :1; // unused

  unsigned int green       :1; // 1, A 
  unsigned int red         :1; // 2, B
  unsigned int blue        :1; // 4, X
  unsigned int yellow      :1; // 3, Y

  // data[4] ------------------
  unsigned int lt          :8; // unknown
  unsigned int rt          :8; // unknown

  // data[6] ------------------
  int x1                   :16; // unused
  int y1                   :16; // unused

  // data[10] -----------------
  int whammy               :16;
  int tilt                 :16;

  // data[14]; ----------------
  unsigned int dummy2      :32; // unused
  unsigned int dummy3      :16; // unused
} __attribute__((__packed__));


struct XBoxMsg
{
  // --------------------------
  unsigned int type       :8;
  unsigned int length     :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int back        :1;

  unsigned int thumb_l     :1;
  unsigned int thumb_r     :1;

  // data[3] ------------------
  unsigned int dummy       :8;
  unsigned int a           :8;
  unsigned int b           :8;
  unsigned int x           :8;
  unsigned int y           :8;
  unsigned int black       :8;
  unsigned int white       :8;
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  int x1                   :16;
  int y1                   :16;

  // data[10] -----------------
  int x2                   :16;
  int y2                   :16;
} __attribute__((__packed__));

enum GamepadType {
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
  char*       name;
};

#endif

/* EOF */
