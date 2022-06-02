/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

namespace xboxdrv {

Xbox360DefaultNames::Xbox360DefaultNames(ControllerMessageDescriptor& desc) :
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

  btn_a = desc.key().put(KeyName("xbox.a"));
  btn_b = desc.key().put(KeyName("xbox.b"));
  btn_x = desc.key().put(KeyName("xbox.x"));
  btn_y = desc.key().put(KeyName("xbox.y"));

  btn_start = desc.key().put(KeyName("xbox.start"));
  btn_guide = desc.key().put(KeyName("xbox.guide"));
  btn_back  = desc.key().put(KeyName("xbox.back"));

  btn_lb = desc.key().put(KeyName("xbox.lb"));
  btn_rb = desc.key().put(KeyName("xbox.rb"));

  btn_lt = desc.key().put(KeyName("xbox.lt"));
  btn_rt = desc.key().put(KeyName("xbox.rt"));

  btn_thumb_l = desc.key().put(KeyName("xbox.thumb_l"));
  btn_thumb_r = desc.key().put(KeyName("xbox.thumb_r"));

  dpad_up    = desc.key().put(KeyName("xbox.dpad_up"));
  dpad_down  = desc.key().put(KeyName("xbox.dpad_down"));
  dpad_left  = desc.key().put(KeyName("xbox.dpad_left"));
  dpad_right = desc.key().put(KeyName("xbox.dpad_right"));

  abs_x1 = desc.abs().put(AbsName("xbox.x1"));
  abs_y1 = desc.abs().put(AbsName("xbox.y1"));
  abs_x2 = desc.abs().put(AbsName("xbox.x2"));
  abs_y2 = desc.abs().put(AbsName("xbox.y2"));

  abs_lt = desc.abs().put(AbsName("xbox.lt"));
  abs_rt = desc.abs().put(AbsName("xbox.rt"));

  abs_a = desc.abs().put(AbsName("xbox.a"));
  abs_b = desc.abs().put(AbsName("xbox.b"));
  abs_x = desc.abs().put(AbsName("xbox.x"));
  abs_y = desc.abs().put(AbsName("xbox.y"));

  abs_black = desc.abs().put(AbsName("xbox.black"));
  abs_white = desc.abs().put(AbsName("xbox.white"));
}

} // namespace xboxdrv

/* EOF */
