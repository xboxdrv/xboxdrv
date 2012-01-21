/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xbox360_default_names.hpp"

#include "controller_message_descriptor.hpp"

Xbox360DefaultNames::Xbox360DefaultNames(ControllerMessageDescriptor& msg_desc) :
  btn_a(-1),
  btn_b(-1),
  btn_x(-1),
  btn_y(-1),
  btn_start(-1),
  btn_guide(-1),
  btn_back(-1),
  btn_thumb_l(-1),
  btn_thumb_r(-1),
  btn_lb(-1),
  btn_rb(-1),
  btn_lt(-1),
  btn_rt(-1),
  dpad_up(-1),
  dpad_down(-1),
  dpad_left(-1),
  dpad_right(-1),
  dpad_x(-1),
  dpad_y(-1),
  abs_x1(-1),
  abs_y1(-1),
  abs_x2(-1),
  abs_y2(-1),
  abs_lt(-1),
  abs_rt(-1),
  abs_a(-1),
  abs_b(-1),
  abs_x(-1),
  abs_y(-1),
  abs_black(-1),
  abs_white(-1)
{
#if 0
  std::string str = to_lower(str_);

  if (str == "start")
    return XBOX_BTN_START;
  else if (str == "guide" || str == "ps")
    return XBOX_BTN_GUIDE;
  else if (str == "back" || str == "select")
    return XBOX_BTN_BACK;

  else if (str == "a" || str == "1" || str == "green" || str == "cross")
    return XBOX_BTN_A;
  else if (str == "b" || str == "2" || str == "red" || str == "circle")
    return XBOX_BTN_B;
  else if (str == "x" || str == "3" || str == "blue" || str == "square")
    return XBOX_BTN_X;
  else if (str == "y" || str == "4" || str == "yellow" || str == "triangle")
    return XBOX_BTN_Y;

  else if (str == "lb" || str == "5" || str == "orange" || str == "white" || str == "l1")
    return XBOX_BTN_LB;
  else if (str == "rb" || str == "6" || str == "black" || str == "r1")
    return XBOX_BTN_RB;

  else if (str == "lt" || str == "7" || str == "l2")
    return XBOX_BTN_LT;
  else if (str == "rt" || str == "8" || str == "r2")
    return XBOX_BTN_RT;

  else if (str == "tl" || str == "l3")
    return XBOX_BTN_THUMB_L;
  else if (str == "tr" || str == "r3")
    return XBOX_BTN_THUMB_R;

  else if (str == "du" || str == "up")
    return XBOX_DPAD_UP;
  else if (str == "dd" || str == "down")
    return XBOX_DPAD_DOWN;
  else if (str == "dl" || str == "left")
    return XBOX_DPAD_LEFT;
  else if (str == "dr" || str == "right")
    return XBOX_DPAD_RIGHT;




  std::string str = to_lower(str_);
  if (str == "x1")
    return XBOX_AXIS_X1;
  else if (str == "y1")
    return XBOX_AXIS_Y1;
  
  else if (str == "x2" || str == "whammy")
    return XBOX_AXIS_X2;
  else if (str == "y2" || str == "tilt")
    return XBOX_AXIS_Y2;
  
  else if (str == "lt" || str == "l2")
    return XBOX_AXIS_LT;
  else if (str == "rt" || str == "r2")
    return XBOX_AXIS_RT;

  else if (str == "dpad_x")
    return XBOX_AXIS_DPAD_X;
  else if (str == "dpad_y")
    return XBOX_AXIS_DPAD_Y;

  else if (str == "trigger" || str == "z" || str == "rudder")
    return XBOX_AXIS_TRIGGER;

  else if (str == "btn_a" || str == "cross")
    return XBOX_AXIS_A;

  else if (str == "btn_b" || str == "circle")
    return XBOX_AXIS_B;

  else if (str == "btn_x" || str == "square")
    return XBOX_AXIS_X;

  else if (str == "btn_y" || str == "triangle")
    return XBOX_AXIS_Y;

  else if (str == "white" || str == "lb"|| str == "l1")
    return XBOX_AXIS_WHITE;

  else if (str == "black" || str == "rb" || str == "r1")
    return XBOX_AXIS_BLACK;

  else if (str == "acc_x")
    return WIIMOTE_ACC_X;

  else if (str == "acc_y")
    return WIIMOTE_ACC_Y;

  else if (str == "acc_z")
    return WIIMOTE_ACC_Z;

  else if (str == "acc_x2")
    return NUNCHUK_ACC_X;

  else if (str == "acc_y2")
    return NUNCHUK_ACC_Y;

  else if (str == "acc_z2")
    return NUNCHUK_ACC_Z;

  else if (str == "ir_x")
    return WIIMOTE_IR_X;

  else if (str == "ir_y")
    return WIIMOTE_IR_Y;

  else if (str == "ir_size")
    return WIIMOTE_IR_SIZE;

  else if (str == "ir_x2")
    return WIIMOTE_IR_X2;

  else if (str == "ir_y2")
    return WIIMOTE_IR_Y2;

  else if (str == "ir_size2")
    return WIIMOTE_IR_SIZE2;

  else if (str == "ir_x3")
    return WIIMOTE_IR_X3;

  else if (str == "ir_y3")
    return WIIMOTE_IR_Y3;

  else if (str == "ir_size3")
    return WIIMOTE_IR_SIZE3;

  else if (str == "ir_x4")
    return WIIMOTE_IR_X4;

  else if (str == "ir_y4")
    return WIIMOTE_IR_Y4;

  else if (str == "ir_size4")
    return WIIMOTE_IR_SIZE2;

#endif

  btn_a = msg_desc.key().put("a");
  btn_b = msg_desc.key().put("b");
  btn_x = msg_desc.key().put("x");
  btn_y = msg_desc.key().put("y");

  btn_start = msg_desc.key().put("start");
  btn_guide = msg_desc.key().put("guide");
  btn_back  = msg_desc.key().put("back");

  btn_lb = msg_desc.key().put("lb");
  btn_rb = msg_desc.key().put("rb");

  btn_lt = msg_desc.key().put("lt");
  btn_rt = msg_desc.key().put("rt");

  btn_thumb_l = msg_desc.key().put("tl"); //, "thumbl");
  btn_thumb_r = msg_desc.key().put("tr"); //, "thumbr");

  dpad_up    = msg_desc.key().put("du"); //, "dpad_up");
  dpad_down  = msg_desc.key().put("dd"); //, "dpad_down");
  dpad_left  = msg_desc.key().put("dl"); //, "dpad_left");
  dpad_right = msg_desc.key().put("dr"); //, "dpad_right");

  dpad_x = msg_desc.abs().put("dpad_x");
  dpad_y = msg_desc.abs().put("dpad_y");

  abs_x1 = msg_desc.abs().put("x1");
  abs_y1 = msg_desc.abs().put("y1");
  abs_x2 = msg_desc.abs().put("x2");
  abs_y2 = msg_desc.abs().put("y2");

  abs_lt = msg_desc.abs().put("lt");
  abs_rt = msg_desc.abs().put("rt");

  abs_a = msg_desc.abs().put("a");
  abs_b = msg_desc.abs().put("b");
  abs_x = msg_desc.abs().put("x");
  abs_y = msg_desc.abs().put("y");
  abs_black = msg_desc.abs().put("black");
  abs_white = msg_desc.abs().put("white");
}

/* EOF */
