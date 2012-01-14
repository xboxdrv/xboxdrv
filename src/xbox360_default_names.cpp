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
  btn_a = msg_desc.register_key("A");
  btn_b = msg_desc.register_key("B");
  btn_x = msg_desc.register_key("X");
  btn_y = msg_desc.register_key("Y");

  btn_start = msg_desc.register_abs("Start");
  btn_guide = msg_desc.register_abs("Guide");
  btn_back  = msg_desc.register_abs("Back");

  btn_lb = msg_desc.register_key("LB");
  btn_rb = msg_desc.register_key("RB");

  btn_thumb_l = msg_desc.register_abs("TL"); //, "ThumbL");
  btn_thumb_r = msg_desc.register_abs("TR"); //, "ThumbR");

  dpad_up    = msg_desc.register_abs("DU"); //, "DPAD_Up");
  dpad_down  = msg_desc.register_abs("DD"); //, "DPAD_Down");
  dpad_left  = msg_desc.register_abs("DL"); //, "DPAD_Left");
  dpad_right = msg_desc.register_abs("DR"); //, "DPAD_Right");

  abs_x1 = msg_desc.register_abs("X1");
  abs_y1 = msg_desc.register_abs("Y1");
  abs_x2 = msg_desc.register_abs("X2");
  abs_y2 = msg_desc.register_abs("Y2");

  abs_lt = msg_desc.register_abs("LT");
  abs_rt = msg_desc.register_abs("RT");
}

/* EOF */
