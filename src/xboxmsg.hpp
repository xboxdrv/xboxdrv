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
  GAMEPAD_FIRESTORM,
  GAMEPAD_FIRESTORM_VSB,
  GAMEPAD_SAITEK_P2500,
  GAMEPAD_SAITEK_P3600,
  GAMEPAD_PLAYSTATION3_USB,
  GAMEPAD_XBOXONE_WIRELESS,
  GAMEPAD_GENERIC_USB
};

enum XboxMsgType {
  XBOX_MSG_XBOX,
  XBOX_MSG_XBOX360,
  XBOX_MSG_PS3USB
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

struct Playstation3USBMsg
{
  unsigned int unknown00 :8; // always 01
  unsigned int unknown01 :8; // always 00

  // 02
  unsigned int select  :1;
  unsigned int l3      :1;
  unsigned int r3      :1;
  unsigned int start   :1;

  unsigned int dpad_up    :1;
  unsigned int dpad_right :1;
  unsigned int dpad_down  :1;
  unsigned int dpad_left  :1;

  // 03
  unsigned int l2   :1;
  unsigned int r2   :1;
  unsigned int l1   :1;
  unsigned int r1   :1;

  unsigned int triangle :1;
  unsigned int circle   :1;
  unsigned int cross    :1;
  unsigned int square   :1;

  // 04
  unsigned int playstation :1;
  unsigned int unknown04   :7;

  unsigned int unknown05 :8; // always 00

  unsigned int x1 :8;
  unsigned int y1 :8;
  unsigned int x2 :8;
  unsigned int y2 :8;

  unsigned int unknown10 :8; // always 00
  unsigned int unknown11 :8; // always 00
  unsigned int unknown12 :8; // always 00
  unsigned int unknown13 :8; // always 00

  unsigned int a_dpad_up    :8;
  unsigned int a_dpad_right :8;
  unsigned int a_dpad_down  :8;
  unsigned int a_dpad_left  :8;

  unsigned int a_l2 :8;
  unsigned int a_r2 :8;
  unsigned int a_l1 :8;
  unsigned int a_r1 :8;

  unsigned int a_triangle :8;
  unsigned int a_circle   :8;
  unsigned int a_cross    :8;
  unsigned int a_square   :8;

  unsigned int unknown26 :8; // always 00
  unsigned int unknown27 :8; // always 00
  unsigned int unknown28 :8; // always 00

  // Bluetooth id start (or something like that)
  unsigned int unknown29 :8;
  unsigned int unknown30 :8;
  unsigned int unknown31 :8;
  unsigned int unknown32 :8;
  unsigned int unknown33 :8;
  unsigned int unknown34 :8;
  unsigned int unknown35 :8;
  unsigned int unknown36 :8;
  unsigned int unknown37 :8;
  unsigned int unknown38 :8;
  unsigned int unknown39 :8;
  unsigned int unknown40 :8;
  // Bluetooth id end

  unsigned int accl_x :16; // little vs big endian!?!
  unsigned int accl_y :16; // little vs big endian!?!
  unsigned int accl_z :16; // little vs big endian!?!

  unsigned int rot_z :16; // very low res (3 or 4 bits), neutral at 5 or 6
} __attribute__((__packed__));

struct XboxGenericMsg
{
  XboxMsgType type;
  union {
    struct Xbox360Msg xbox360;
    struct XboxMsg    xbox;
    struct Playstation3USBMsg ps3usb;
  };
};

std::ostream& operator<<(std::ostream& out, const GamepadType& type);
std::ostream& operator<<(std::ostream& out, const Xbox360Msg& msg);
std::ostream& operator<<(std::ostream& out, const XboxMsg& msg);
std::ostream& operator<<(std::ostream& out, const Playstation3USBMsg& msg);
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

int  get_button(XboxGenericMsg& msg, XboxButton button);
void set_button(XboxGenericMsg& msg, XboxButton button, bool v);
int  get_axis(XboxGenericMsg& msg, XboxAxis axis);
void set_axis(XboxGenericMsg& msg, XboxAxis axis, int v);
float get_axis_float(XboxGenericMsg& msg, XboxAxis axis);
void  set_axis_float(XboxGenericMsg& msg, XboxAxis axis, float v);

XboxButton string2btn(const std::string& str_);
XboxAxis   string2axis(const std::string& str_);
std::string btn2string(XboxButton btn);
std::string axis2string(XboxAxis axis);

int get_axis_min(XboxAxis axis);
int get_axis_max(XboxAxis axis);

std::string gamepadtype_to_string(const GamepadType& type);
std::string gamepadtype_to_macro_string(const GamepadType& type);

#endif

/* EOF */
