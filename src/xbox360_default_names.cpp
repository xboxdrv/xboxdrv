/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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

  btn_a = desc.key().put("a");
  btn_b = desc.key().put("b");
  btn_x = desc.key().put("x");
  btn_y = desc.key().put("y");

  btn_start = desc.key().put("start");
  btn_guide = desc.key().put("guide");
  btn_back  = desc.key().put("back");

  btn_lb = desc.key().put("lb");
  btn_rb = desc.key().put("rb");

  btn_lt = desc.key().put("lt");
  btn_rt = desc.key().put("rt");

  btn_thumb_l = desc.key().put("thumb_l");
  btn_thumb_r = desc.key().put("thumb_r");

  dpad_up    = desc.key().put("dpad_up");
  dpad_down  = desc.key().put("dpad_down");
  dpad_left  = desc.key().put("dpad_left");
  dpad_right = desc.key().put("dpad_right");

  abs_x1 = desc.abs().put("x1");
  abs_y1 = desc.abs().put("y1");
  abs_x2 = desc.abs().put("x2");
  abs_y2 = desc.abs().put("y2");

  abs_lt = desc.abs().put("lt");
  abs_rt = desc.abs().put("rt");

  abs_a = desc.abs().put("a");
  abs_b = desc.abs().put("b");
  abs_x = desc.abs().put("x");
  abs_y = desc.abs().put("y");

  abs_black = desc.abs().put("black");
  abs_white = desc.abs().put("white");

  // Config-facing aliases (examples use A=..., du=..., tl=..., ...)
  desc.key().alias("A", btn_a);
  desc.key().alias("B", btn_b);
  desc.key().alias("X", btn_x);
  desc.key().alias("Y", btn_y);
  desc.key().alias("LB", btn_lb);
  desc.key().alias("RB", btn_rb);
  desc.key().alias("LT", btn_lt);
  desc.key().alias("RT", btn_rt);
  desc.key().alias("tl", btn_thumb_l);
  desc.key().alias("tr", btn_thumb_r);
  desc.key().alias("du", dpad_up);
  desc.key().alias("dd", dpad_down);
  desc.key().alias("dl", dpad_left);
  desc.key().alias("dr", dpad_right);
  // Historical string2btn names (PR #227 / issue #225): btn2string used to
  // print DPAD_*; short up/down/left/right were also accepted.
  desc.key().alias("up", dpad_up);
  desc.key().alias("down", dpad_down);
  desc.key().alias("left", dpad_left);
  desc.key().alias("right", dpad_right);
  desc.key().alias("DPAD_UP", dpad_up);
  desc.key().alias("DPAD_DOWN", dpad_down);
  desc.key().alias("DPAD_LEFT", dpad_left);
  desc.key().alias("DPAD_RIGHT", dpad_right);

  // Default uinput maps (UInputOptions::set_defaults / mimic_xpad) use the
  // "gamepad." prefix; register those names so ButtonMap::init() can resolve.
  desc.key().alias("gamepad.a", btn_a);
  desc.key().alias("gamepad.b", btn_b);
  desc.key().alias("gamepad.x", btn_x);
  desc.key().alias("gamepad.y", btn_y);
  desc.key().alias("gamepad.start", btn_start);
  desc.key().alias("gamepad.guide", btn_guide);
  desc.key().alias("gamepad.back", btn_back);
  desc.key().alias("gamepad.lb", btn_lb);
  desc.key().alias("gamepad.rb", btn_rb);
  desc.key().alias("gamepad.lt", btn_lt);
  desc.key().alias("gamepad.rt", btn_rt);
  desc.key().alias("gamepad.tl", btn_thumb_l);
  desc.key().alias("gamepad.tr", btn_thumb_r);
  desc.key().alias("gamepad.dpad_up", dpad_up);
  desc.key().alias("gamepad.dpad_down", dpad_down);
  desc.key().alias("gamepad.dpad_left", dpad_left);
  desc.key().alias("gamepad.dpad_right", dpad_right);

  desc.abs().alias("X1", abs_x1);
  desc.abs().alias("Y1", abs_y1);
  desc.abs().alias("X2", abs_x2);
  desc.abs().alias("Y2", abs_y2);
  desc.abs().alias("LT", abs_lt);
  desc.abs().alias("RT", abs_rt);

  desc.abs().alias("gamepad.x1", abs_x1);
  desc.abs().alias("gamepad.y1", abs_y1);
  desc.abs().alias("gamepad.x2", abs_x2);
  desc.abs().alias("gamepad.y2", abs_y2);
  desc.abs().alias("gamepad.lt", abs_lt);
  desc.abs().alias("gamepad.rt", abs_rt);
  // dpad_x / dpad_y are created by CompatModifier when missing; alias once present
  // is handled there. Pre-register names so default axis maps can resolve early.
  int dpad_x = desc.abs().getput("dpad_x");
  int dpad_y = desc.abs().getput("dpad_y");
  desc.abs().alias("gamepad.dpad_x", dpad_x);
  desc.abs().alias("gamepad.dpad_y", dpad_y);
}

} // namespace xboxdrv

/* EOF */
