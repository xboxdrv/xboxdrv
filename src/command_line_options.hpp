/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_COMMAND_LINE_OPTIONS_HPP
#define HEADER_COMMAND_LINE_OPTIONS_HPP

#include <vector>
#include <map>

#include "modifier.hpp"
#include "xboxmsg.hpp"
#include "uinput.hpp"
#include "arg_parser.hpp"

class Xboxdrv;

class CommandLineOptions 
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
  
  char busid[4];
  char devid[4];

  int vendor_id;
  int product_id;

  uInputCfg uinput_config;
  int deadzone;
  int deadzone_trigger;
  std::vector<ButtonMapping> button_map;
  std::vector<AxisMapping>   axis_map;
  std::vector<AutoFireMapping> autofire_map;
  std::vector<RelativeAxisMapping> relative_axis_map;
  std::vector<CalibrationMapping> calibration_map;
  std::vector<AxisSensitivityMapping> axis_sensitivity_map;
  bool square_axis;
  bool four_way_restrictor;
  int  dpad_rotation;
  std::string evdev_device;
  std::map<int, XboxAxis>   evdev_absmap;
  std::map<int, XboxButton> evdev_keymap;

  ArgParser argp;

public:
  CommandLineOptions();
  void parse_args(int argc, char** argv);

  void print_help() const;
  void print_led_help() const;
  void print_version() const;
};

extern CommandLineOptions* command_line_options;

#endif

/* EOF */
