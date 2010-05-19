/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include <linux/input.h>

#include "uinput_cfg.hpp"

uInputCfg::uInputCfg() :
  device_name("Xbox Gamepad (userspace driver)"),
  trigger_as_button(false),
  dpad_as_button(false),
  trigger_as_zaxis(false),
  dpad_only(false),
  force_feedback(false),
  extra_devices(true)
{
  for(int i = 0; i < XBOX_BTN_MAX; ++i)
    std::fill_n(btn_map[i],  static_cast<int>(XBOX_BTN_MAX),  ButtonEvent::invalid());
  std::fill_n(axis_map, static_cast<int>(XBOX_AXIS_MAX), AxisEvent::invalid());

  ButtonEvent* bmap = btn_map[XBOX_BTN_UNKNOWN]; // FIXME: little hacky to abuse XBOX_BTN_UNKNOWN

  // Button Mapping
  bmap[XBOX_BTN_START] = ButtonEvent::create(EV_KEY, BTN_START);
  bmap[XBOX_BTN_GUIDE] = ButtonEvent::create(EV_KEY, BTN_MODE);
  bmap[XBOX_BTN_BACK]  = ButtonEvent::create(EV_KEY, BTN_SELECT);

  bmap[XBOX_BTN_A] = ButtonEvent::create(EV_KEY, BTN_A);
  bmap[XBOX_BTN_B] = ButtonEvent::create(EV_KEY, BTN_B);
  bmap[XBOX_BTN_X] = ButtonEvent::create(EV_KEY, BTN_X);
  bmap[XBOX_BTN_Y] = ButtonEvent::create(EV_KEY, BTN_Y);

  bmap[XBOX_BTN_GREEN]  = ButtonEvent::create(EV_KEY, BTN_0);
  bmap[XBOX_BTN_RED]    = ButtonEvent::create(EV_KEY, BTN_1);
  bmap[XBOX_BTN_YELLOW] = ButtonEvent::create(EV_KEY, BTN_2);
  bmap[XBOX_BTN_BLUE]   = ButtonEvent::create(EV_KEY, BTN_3);
  bmap[XBOX_BTN_ORANGE] = ButtonEvent::create(EV_KEY, BTN_4);

  bmap[XBOX_BTN_WHITE] = ButtonEvent::create(EV_KEY, BTN_TL);
  bmap[XBOX_BTN_BLACK] = ButtonEvent::create(EV_KEY, BTN_TR);

  bmap[XBOX_BTN_LB] = ButtonEvent::create(EV_KEY, BTN_TL);
  bmap[XBOX_BTN_RB] = ButtonEvent::create(EV_KEY, BTN_TR);

  bmap[XBOX_BTN_LT] = ButtonEvent::create(EV_KEY, BTN_TL2);
  bmap[XBOX_BTN_RT] = ButtonEvent::create(EV_KEY, BTN_TR2);

  bmap[XBOX_BTN_THUMB_L] = ButtonEvent::create(EV_KEY, BTN_THUMBL);
  bmap[XBOX_BTN_THUMB_R] = ButtonEvent::create(EV_KEY, BTN_THUMBR);
  
  bmap[XBOX_DPAD_UP]    = ButtonEvent::create(EV_KEY, BTN_BASE);
  bmap[XBOX_DPAD_DOWN]  = ButtonEvent::create(EV_KEY, BTN_BASE2);
  bmap[XBOX_DPAD_LEFT]  = ButtonEvent::create(EV_KEY, BTN_BASE3);
  bmap[XBOX_DPAD_RIGHT] = ButtonEvent::create(EV_KEY, BTN_BASE4);

  // Axis Mapping
  axis_map[XBOX_AXIS_X1]      = AxisEvent::create(EV_ABS, ABS_X); 
  axis_map[XBOX_AXIS_Y1]      = AxisEvent::create(EV_ABS, ABS_Y); 
  axis_map[XBOX_AXIS_X2]      = AxisEvent::create(EV_ABS, ABS_RX);
  axis_map[XBOX_AXIS_Y2]      = AxisEvent::create(EV_ABS, ABS_RY);
  axis_map[XBOX_AXIS_LT]      = AxisEvent::create(EV_ABS, ABS_BRAKE);
  axis_map[XBOX_AXIS_RT]      = AxisEvent::create(EV_ABS, ABS_GAS); 
  axis_map[XBOX_AXIS_TRIGGER] = AxisEvent::create(EV_ABS, ABS_Z);
  axis_map[XBOX_AXIS_DPAD_X]  = AxisEvent::create(EV_ABS, ABS_HAT0X);
  axis_map[XBOX_AXIS_DPAD_Y]  = AxisEvent::create(EV_ABS, ABS_HAT0Y);
}

void
uInputCfg::mimic_xpad()
{
  device_name = "Microsoft X-Box 360 pad";

  extra_devices = false;

  ButtonEvent* bmap = btn_map[XBOX_BTN_UNKNOWN]; // FIXME: little hacky to abuse XBOX_BTN_UNKNOWN

  bmap[XBOX_BTN_START] = ButtonEvent::create(EV_KEY, BTN_START);
  bmap[XBOX_BTN_GUIDE] = ButtonEvent::create(EV_KEY, BTN_MODE);
  bmap[XBOX_BTN_BACK]  = ButtonEvent::create(EV_KEY, BTN_BACK);

  bmap[XBOX_BTN_A] = ButtonEvent::create(EV_KEY, BTN_A);
  bmap[XBOX_BTN_B] = ButtonEvent::create(EV_KEY, BTN_B);
  bmap[XBOX_BTN_X] = ButtonEvent::create(EV_KEY, BTN_X);
  bmap[XBOX_BTN_Y] = ButtonEvent::create(EV_KEY, BTN_Y);

  bmap[XBOX_BTN_GREEN]  = ButtonEvent::create(EV_KEY, BTN_0);
  bmap[XBOX_BTN_RED]    = ButtonEvent::create(EV_KEY, BTN_1);
  bmap[XBOX_BTN_YELLOW] = ButtonEvent::create(EV_KEY, BTN_2);
  bmap[XBOX_BTN_BLUE]   = ButtonEvent::create(EV_KEY, BTN_3);
  bmap[XBOX_BTN_ORANGE] = ButtonEvent::create(EV_KEY, BTN_4);

  bmap[XBOX_BTN_WHITE] = ButtonEvent::create(EV_KEY, BTN_TL);
  bmap[XBOX_BTN_BLACK] = ButtonEvent::create(EV_KEY, BTN_TR);

  bmap[XBOX_BTN_LB] = ButtonEvent::create(EV_KEY, BTN_TL);
  bmap[XBOX_BTN_RB] = ButtonEvent::create(EV_KEY, BTN_TR);
            
  bmap[XBOX_BTN_LT] = ButtonEvent::create(EV_KEY, BTN_TL2);
  bmap[XBOX_BTN_RT] = ButtonEvent::create(EV_KEY, BTN_TR2);
            
  bmap[XBOX_BTN_THUMB_L] = ButtonEvent::create(EV_KEY, BTN_THUMBL);
  bmap[XBOX_BTN_THUMB_R] = ButtonEvent::create(EV_KEY, BTN_THUMBR);
            
  bmap[XBOX_DPAD_UP]    = ButtonEvent::create(EV_KEY, BTN_BASE);
  bmap[XBOX_DPAD_DOWN]  = ButtonEvent::create(EV_KEY, BTN_BASE2);
  bmap[XBOX_DPAD_LEFT]  = ButtonEvent::create(EV_KEY, BTN_BASE3);
  bmap[XBOX_DPAD_RIGHT] = ButtonEvent::create(EV_KEY, BTN_BASE4);

  // Axis Mapping
  axis_map[XBOX_AXIS_X1]      = AxisEvent::create(EV_ABS, ABS_X, 16, 128);
  axis_map[XBOX_AXIS_Y1]      = AxisEvent::create(EV_ABS, ABS_Y, 16, 128);
  axis_map[XBOX_AXIS_X2]      = AxisEvent::create(EV_ABS, ABS_RX, 16, 128);
  axis_map[XBOX_AXIS_Y2]      = AxisEvent::create(EV_ABS, ABS_RY, 16, 128);
  axis_map[XBOX_AXIS_LT]      = AxisEvent::create(EV_ABS, ABS_Z);
  axis_map[XBOX_AXIS_RT]      = AxisEvent::create(EV_ABS, ABS_RZ);
  axis_map[XBOX_AXIS_TRIGGER] = AxisEvent::create(EV_ABS, ABS_Z);
  axis_map[XBOX_AXIS_DPAD_X]  = AxisEvent::create(EV_ABS, ABS_HAT0X);
  axis_map[XBOX_AXIS_DPAD_Y]  = AxisEvent::create(EV_ABS, ABS_HAT0Y);
}

/* EOF */
