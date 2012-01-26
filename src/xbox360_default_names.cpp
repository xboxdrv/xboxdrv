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
  else if (str == "trigger" || str == "z" || str == "rudder")
    return XBOX_AXIS_TRIGGER;
#endif

  btn_a = msg_desc.key().put("a", "cross", "1", "green");
  btn_b = msg_desc.key().put("b", "circle", "2", "red");
  btn_x = msg_desc.key().put("x", "square", "3", "blue");
  btn_y = msg_desc.key().put("y", "triangle", "4", "yellow");

  btn_start = msg_desc.key().put("start");
  btn_guide = msg_desc.key().put("guide", "ps");
  btn_back  = msg_desc.key().put("back", "select");

  btn_lb = msg_desc.key().put("lb", "white", "l1", "orange", "5");
  btn_rb = msg_desc.key().put("rb", "black", "r1", "6");

  btn_lt = msg_desc.key().put("lt", "7", "l2");
  btn_rt = msg_desc.key().put("rt", "7", "l2");

  btn_thumb_l = msg_desc.key().put("tl", "thumbl", "l3");
  btn_thumb_r = msg_desc.key().put("tr", "thumbr", "r3");

  dpad_up    = msg_desc.key().put("du", "dpad_up", "up");
  dpad_down  = msg_desc.key().put("dd", "dpad_down", "down");
  dpad_left  = msg_desc.key().put("dl", "dpad_left", "left");
  dpad_right = msg_desc.key().put("dr", "dpad_right", "right");

  dpad_x = msg_desc.abs().put("dpad_x");
  dpad_y = msg_desc.abs().put("dpad_y");

  abs_x1 = msg_desc.abs().put("x1");
  abs_y1 = msg_desc.abs().put("y1");
  abs_x2 = msg_desc.abs().put("x2", "whammy");
  abs_y2 = msg_desc.abs().put("y2", "tilt");

  abs_lt = msg_desc.abs().put("lt", "7", "l2");
  abs_rt = msg_desc.abs().put("rt", "8", "r2");

  abs_a = msg_desc.abs().put("a", "cross");
  abs_b = msg_desc.abs().put("b", "circle");
  abs_x = msg_desc.abs().put("x", "square");
  abs_y = msg_desc.abs().put("y", "triangle");
  abs_black = msg_desc.abs().put("black");
  abs_white = msg_desc.abs().put("white");
}

/* EOF */
