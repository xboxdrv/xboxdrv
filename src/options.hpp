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

#ifndef HEADER_XBOXDRV_OPTIONS_HPP
#define HEADER_XBOXDRV_OPTIONS_HPP

#include <string>
#include <map>
#include <vector>

#include "evdev_absmap.hpp"
#include "modifier.hpp"
#include "uinput_cfg.hpp"
#include "xpad_device.hpp"

#include "modifier/autofire_modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/axis_sensitivty_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/calibration_modifier.hpp"
#include "modifier/relativeaxis_modifier.hpp"

class ControllerOptions
{
public:
  ControllerOptions();

  uInputCfg uinput;
  std::vector<ModifierPtr> modifier;
};

class Options
{
public:
  enum { RUN_DEFAULT,
         RUN_DAEMON, 
         RUN_LIST_CONTROLLER,
         RUN_LIST_SUPPORTED_DEVICES,
         RUN_LIST_SUPPORTED_DEVICES_XPAD,
         PRINT_VERSION,
         PRINT_HELP,
         PRINT_HELP_DEVICES,
         PRINT_LED_HELP
  } mode;

  // General program options
  bool verbose;
  bool silent;
  bool quiet;
  bool rumble;
  int  led;
  int  rumble_l;
  int  rumble_r;
  int  rumble_gain;
  int  controller_id;
  int  wireless_id;
  bool instant_exit;
  bool no_uinput;
  bool detach_kernel_driver;

  GamepadType gamepad_type;
  
  // device options
  std::string busid;
  std::string devid;

  int vendor_id;
  int product_id;

  std::string evdev_device;
  EvdevAbsMap evdev_absmap;
  bool evdev_grab;
  bool evdev_debug;
  std::map<int, XboxButton> evdev_keymap;

  // controller options
  ControllerOptions controller;

  // chatpad options
  bool chatpad;
  bool chatpad_no_init;
  bool chatpad_debug;

  // headset options
  bool headset;
  bool headset_debug;
  std::string headset_dump;
  std::string headset_play;

  // daemon options
  bool detach;
  std::string pid_file;

  std::vector<std::string> exec;

  Options();
};

extern Options* g_options;

#endif

/* EOF */
