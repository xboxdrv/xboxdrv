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

#include "arg_parser.hpp"
#include "axis_map_option.hpp"
#include "button_map_option.hpp"
#include "ini_schema.hpp"
#include "uinput.hpp"

class Options;
class Xboxdrv;
class AxisMap;

class CommandLineParser 
{
public:
  ArgParser m_argp;
  INISchema m_ini;
  Options*  m_options;
  std::vector<std::string> m_directory_context;
  
public:
  CommandLineParser();

  void parse_args(int argc, char** argv, Options* options);

  void print_help() const;
  void print_led_help() const;
  void print_version() const;
  void create_ini_schema(Options* opts);

private:
  void set_device_name(const std::string& name, const std::string& value);
  void set_device_usbid(const std::string& name, const std::string& value);

  void set_absmap(AxisMapOptions& axis_map, const std::string& name, const std::string& value);
  void set_absmap(const std::string& name, const std::string& value);
  void set_keymap(ButtonMapOptions& btn_map, const std::string& name, const std::string& value);
  void set_keymap(const std::string& name, const std::string& value);
  void set_relmap(const std::string& name, const std::string& value);

  void set_modifier(const std::string& name, const std::string& value);

  void set_axismap(const std::string& name, const std::string& value);
  void set_buttonmap(const std::string& name, const std::string& value);

  void set_relative_axis(const std::string& name, const std::string& value);
  void set_autofire(const std::string& name, const std::string& value);
  void set_calibration(const std::string& name, const std::string& value);
  void set_axis_sensitivity(const std::string& name, const std::string& value);


  void set_absmap_n(int controller, int config, const std::string& name, const std::string& value);
  void set_keymap_n(int controller, int config, const std::string& name, const std::string& value);
  void set_relmap_n(int controller, int config, const std::string& name, const std::string& value);
  void set_modifier_n(int controller, int config, const std::string& name, const std::string& value);

  void set_axismap_n(int controller, int config, const std::string& name, const std::string& value);
  void set_buttonmap_n(int controller, int config, const std::string& name, const std::string& value);

  void set_relative_axis_n(int controller, int config, const std::string& name, const std::string& value);
  void set_autofire_n(int controller, int config, const std::string& name, const std::string& value);
  void set_calibration_n(int controller, int config, const std::string& name, const std::string& value);
  void set_axis_sensitivity_n(int controller, int config, const std::string& name, const std::string& value);


  void set_deadzone(const std::string& value);
  void set_deadzone_trigger(const std::string& value);
  void set_square_axis();
  void set_four_way_restrictor();
  void set_dpad_rotation(const std::string& value);

  void set_evdev_absmap(const std::string& name, const std::string& value);
  void set_evdev_keymap(const std::string& name, const std::string& value);
  void set_evdev_relmap(const std::string& name, const std::string& value);

  void read_buildin_config_file(const std::string& filename, 
                                const char* data, unsigned int data_len);
  void read_config_file(const std::string& filename);
  void read_alt_config_file(const std::string& filename);

  void mouse();

  void set_generic_usb_spec(const std::string& name);

private:
  void init_argp();
  void init_ini(Options* opts);
  std::string get_directory_context() const;

private:
  CommandLineParser(const CommandLineParser&);
  CommandLineParser& operator=(const CommandLineParser&);
};

extern Options* g_options;

#endif

/* EOF */
