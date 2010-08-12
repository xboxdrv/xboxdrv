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
  trigger_as_zaxis(false),
  dpad_as_button(false),
  dpad_only(false),
  force_feedback(false),
  extra_devices(true)
{
  btn_map.clear();
  std::fill_n(axis_map, static_cast<int>(XBOX_AXIS_MAX), AxisEvent::invalid());

  // Button Mapping
  btn_map.bind(XBOX_BTN_START, ButtonEvent::create_key(BTN_START));
  btn_map.bind(XBOX_BTN_GUIDE, ButtonEvent::create_key(BTN_MODE));
  btn_map.bind(XBOX_BTN_BACK, ButtonEvent::create_key(BTN_SELECT));

  btn_map.bind(XBOX_BTN_A, ButtonEvent::create_key(BTN_A));
  btn_map.bind(XBOX_BTN_B, ButtonEvent::create_key(BTN_B));
  btn_map.bind(XBOX_BTN_X, ButtonEvent::create_key(BTN_X));
  btn_map.bind(XBOX_BTN_Y, ButtonEvent::create_key(BTN_Y));

  btn_map.bind(XBOX_BTN_GREEN, ButtonEvent::create_key(BTN_0));
  btn_map.bind(XBOX_BTN_RED, ButtonEvent::create_key(BTN_1));
  btn_map.bind(XBOX_BTN_YELLOW, ButtonEvent::create_key(BTN_2));
  btn_map.bind(XBOX_BTN_BLUE, ButtonEvent::create_key(BTN_3));
  btn_map.bind(XBOX_BTN_ORANGE, ButtonEvent::create_key(BTN_4));

  btn_map.bind(XBOX_BTN_WHITE, ButtonEvent::create_key(BTN_TL));
  btn_map.bind(XBOX_BTN_BLACK, ButtonEvent::create_key(BTN_TR));

  btn_map.bind(XBOX_BTN_LB, ButtonEvent::create_key(BTN_TL));
  btn_map.bind(XBOX_BTN_RB, ButtonEvent::create_key(BTN_TR));

  btn_map.bind(XBOX_BTN_LT, ButtonEvent::create_key(BTN_TL2));
  btn_map.bind(XBOX_BTN_RT, ButtonEvent::create_key(BTN_TR2));

  btn_map.bind(XBOX_BTN_THUMB_L, ButtonEvent::create_key(BTN_THUMBL));
  btn_map.bind(XBOX_BTN_THUMB_R, ButtonEvent::create_key(BTN_THUMBR));
  
  btn_map.bind(XBOX_DPAD_UP, ButtonEvent::create_key(BTN_BASE));
  btn_map.bind(XBOX_DPAD_DOWN, ButtonEvent::create_key(BTN_BASE2));
  btn_map.bind(XBOX_DPAD_LEFT, ButtonEvent::create_key(BTN_BASE3));
  btn_map.bind(XBOX_DPAD_RIGHT, ButtonEvent::create_key(BTN_BASE4));

  // Axis Mapping
  axis_map[XBOX_AXIS_X1]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_X, -32768, 32767, 0, 0); 
  axis_map[XBOX_AXIS_Y1]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_Y, -32768, 32767, 0, 0); 
  axis_map[XBOX_AXIS_X2]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_RX, -32768, 32767, 0, 0);
  axis_map[XBOX_AXIS_Y2]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_RY, -32768, 32767, 0, 0);
  axis_map[XBOX_AXIS_LT]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_BRAKE, 0, 255, 0, 0);
  axis_map[XBOX_AXIS_RT]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_GAS, 0, 255, 0, 0); 
  axis_map[XBOX_AXIS_TRIGGER] = AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z, -255, 255, 0, 0);
  axis_map[XBOX_AXIS_DPAD_X]  = AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0X, -1, 1, 0, 0);
  axis_map[XBOX_AXIS_DPAD_Y]  = AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0Y, -1, 1, 0, 0);
}

void
uInputCfg::mimic_xpad()
{
  device_name = "Microsoft X-Box 360 pad";

  extra_devices = false;

  btn_map.bind(XBOX_BTN_START, ButtonEvent::create_key(BTN_START));
  btn_map.bind(XBOX_BTN_GUIDE, ButtonEvent::create_key(BTN_MODE));
  btn_map.bind(XBOX_BTN_BACK, ButtonEvent::create_key(BTN_BACK));

  btn_map.bind(XBOX_BTN_A, ButtonEvent::create_key(BTN_A));
  btn_map.bind(XBOX_BTN_B, ButtonEvent::create_key(BTN_B));
  btn_map.bind(XBOX_BTN_X, ButtonEvent::create_key(BTN_X));
  btn_map.bind(XBOX_BTN_Y, ButtonEvent::create_key(BTN_Y));

  btn_map.bind(XBOX_BTN_GREEN, ButtonEvent::create_key(BTN_0));
  btn_map.bind(XBOX_BTN_RED, ButtonEvent::create_key(BTN_1));
  btn_map.bind(XBOX_BTN_YELLOW, ButtonEvent::create_key(BTN_2));
  btn_map.bind(XBOX_BTN_BLUE, ButtonEvent::create_key(BTN_3));
  btn_map.bind(XBOX_BTN_ORANGE, ButtonEvent::create_key(BTN_4));

  btn_map.bind(XBOX_BTN_WHITE, ButtonEvent::create_key(BTN_TL));
  btn_map.bind(XBOX_BTN_BLACK, ButtonEvent::create_key(BTN_TR));

  btn_map.bind(XBOX_BTN_LB, ButtonEvent::create_key(BTN_TL));
  btn_map.bind(XBOX_BTN_RB, ButtonEvent::create_key(BTN_TR));
            
  btn_map.bind(XBOX_BTN_LT, ButtonEvent::create_key(BTN_TL2));
  btn_map.bind(XBOX_BTN_RT, ButtonEvent::create_key(BTN_TR2));
            
  btn_map.bind(XBOX_BTN_THUMB_L, ButtonEvent::create_key(BTN_THUMBL));
  btn_map.bind(XBOX_BTN_THUMB_R, ButtonEvent::create_key(BTN_THUMBR));
            
  btn_map.bind(XBOX_DPAD_UP, ButtonEvent::create_key(BTN_BASE));
  btn_map.bind(XBOX_DPAD_DOWN, ButtonEvent::create_key(BTN_BASE2));
  btn_map.bind(XBOX_DPAD_LEFT, ButtonEvent::create_key(BTN_BASE3));
  btn_map.bind(XBOX_DPAD_RIGHT, ButtonEvent::create_key(BTN_BASE4));

  // Axis Mapping
  axis_map[XBOX_AXIS_X1]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_X,  -32768, 32767, 16, 128);
  axis_map[XBOX_AXIS_Y1]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_Y,  -32768, 32767, 16, 128);
  axis_map[XBOX_AXIS_X2]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_RX, -32768, 32767, 16, 128);
  axis_map[XBOX_AXIS_Y2]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_RY, -32768, 32767, 16, 128);
  axis_map[XBOX_AXIS_LT]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z,  0, 255, 0, 0);
  axis_map[XBOX_AXIS_RT]      = AxisEvent::create_abs(DEVICEID_AUTO, ABS_RZ, 0, 255, 0, 0);
  axis_map[XBOX_AXIS_TRIGGER] = AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z, -255, 255, 0, 0);
  axis_map[XBOX_AXIS_DPAD_X]  = AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0X, -1, 1, 0, 0);
  axis_map[XBOX_AXIS_DPAD_Y]  = AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0Y, -1, 1, 0, 0);
}

/* EOF */
