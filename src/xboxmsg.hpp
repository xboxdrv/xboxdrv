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
  GAMEPAD_XBOX360_GUITAR
};

struct Xbox360Msg
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
  unsigned int back        :1;

  unsigned int thumb_l     :1;
  unsigned int thumb_r     :1;

  // data[3] ------------------
  unsigned int lb          :1;
  unsigned int rb          :1;
  unsigned int guide       :1;
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

struct Xbox360GuitarMsg
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
  unsigned int back        :1;

  unsigned int thumb_l     :1; // unused
  unsigned int thumb_r     :1; // unused

  // data[3] ------------------
  unsigned int orange      :1; // 5
  unsigned int rb          :1; // unused
  unsigned int guide       :1; 
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

struct XboxMsg
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


struct XboxGenericMsg
{
  GamepadType type;
  union {
    struct Xbox360GuitarMsg guitar;
    struct Xbox360Msg       xbox360;
    struct XboxMsg          xbox;
  };
};

std::ostream& operator<<(std::ostream& out, const GamepadType& type);
std::ostream& operator<<(std::ostream& out, const Xbox360GuitarMsg& msg);
std::ostream& operator<<(std::ostream& out, const Xbox360Msg& msg);
std::ostream& operator<<(std::ostream& out, const XboxMsg& msg);
std::ostream& operator<<(std::ostream& out, const XboxGenericMsg& msg);

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

  XBOX_AXIS_MAX
};

int  get_button(XboxGenericMsg& msg, XboxButton button);
void set_button(XboxGenericMsg& msg, XboxButton button, int v);
int  get_axis(XboxGenericMsg& msg, XboxAxis axis);
void set_axis(XboxGenericMsg& msg, XboxAxis axis, int v);

XboxButton string2btn(const std::string& str_);
XboxAxis   string2axis(const std::string& str_);
std::string btn2string(XboxButton btn);
std::string axis2string(XboxAxis axis);

#endif

/* EOF */
