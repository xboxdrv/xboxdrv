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
  btn_a(),
  btn_b(),
  btn_x(),
  btn_y(),
  btn_start(),
  btn_guide(),
  btn_back(),
  btn_thumb_l(),
  btn_thumb_r(),
  btn_lb(),
  btn_rb(),
  dpad_up(),
  dpad_down(),
  dpad_left(),
  dpad_right(),
  abs_x1(),
  abs_y1(),
  abs_x2(),
  abs_y2(),
  abs_lt(),
  abs_rt()
{
  btn_a = msg_desc.key().put("a");
  btn_b = msg_desc.key().put("b");
  btn_x = msg_desc.key().put("x");
  btn_y = msg_desc.key().put("y");

  btn_start = msg_desc.key().put("start");
  btn_guide = msg_desc.key().put("guide");
  btn_back  = msg_desc.key().put("back");

  btn_lb = msg_desc.key().put("lb");
  btn_rb = msg_desc.key().put("rb");

  btn_thumb_l = msg_desc.key().put("tl"); //, "thumbl");
  btn_thumb_r = msg_desc.key().put("tr"); //, "thumbr");

  dpad_up    = msg_desc.key().put("du"); //, "dpad_up");
  dpad_down  = msg_desc.key().put("dd"); //, "dpad_down");
  dpad_left  = msg_desc.key().put("dl"); //, "dpad_left");
  dpad_right = msg_desc.key().put("dr"); //, "dpad_right");

  abs_x1 = msg_desc.abs().put("x1");
  abs_y1 = msg_desc.abs().put("y1");
  abs_x2 = msg_desc.abs().put("x2");
  abs_y2 = msg_desc.abs().put("y2");

  abs_lt = msg_desc.abs().put("lt");
  abs_rt = msg_desc.abs().put("rt");
}

/* EOF */
