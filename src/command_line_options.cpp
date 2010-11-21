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

#include "command_line_options.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "arg_parser.hpp"
#include "helper.hpp"
#include "ini_schema_builder.hpp"
#include "ini_parser.hpp"
#include "options.hpp"
#include "uinput_deviceid.hpp"

#define RAISE_EXCEPTION(x) do {                         \
    std::ostringstream kiJk8f08d4oMX;                   \
    kiJk8f08d4oMX << x;                                 \
    throw std::runtime_error(kiJk8f08d4oMX.str());      \
  } while(0)

Options* g__options = 0;

enum {
  OPTION_HELP,
  OPTION_VERBOSE,
  OPTION_VERSION,
  OPTION_QUIET,
  OPTION_SILENT,
  OPTION_DAEMON,
  OPTION_CONFIG,
  OPTION_WRITE_CONFIG,
  OPTION_TEST_RUMBLE,
  OPTION_RUMBLE,
  OPTION_QUIT,
  OPTION_NO_UINPUT,
  OPTION_MIMIC_XPAD,
  OPTION_NO_EXTRA_DEVICES,
  OPTION_TYPE,
  OPTION_FORCE_FEEDBACK,
  OPTION_RUMBLE_GAIN,
  OPTION_BUTTONMAP,
  OPTION_AXISMAP,
  OPTION_NAME,
  OPTION_UI_CLEAR,
  OPTION_UI_AXISMAP,
  OPTION_UI_BUTTONMAP,
  OPTION_ID,
  OPTION_WID,
  OPTION_LED,
  OPTION_DPAD_ONLY,
  OPTION_DPAD_AS_BUTTON,
  OPTION_DEADZONE,
  OPTION_DEADZONE_TRIGGER,
  OPTION_TRIGGER_AS_BUTTON,
  OPTION_TRIGGER_AS_ZAXIS,
  OPTION_AUTOFIRE,
  OPTION_CALIBRARIOTION,
  OPTION_RELATIVE_AXIS,
  OPTION_SQUARE_AXIS,
  OPTION_FOUR_WAY_RESTRICTOR,
  OPTION_DPAD_ROTATION,
  OPTION_AXIS_SENSITIVITY,
  OPTION_HELP_LED,
  OPTION_DEVICE_BY_ID,
  OPTION_DEVICE_BY_PATH,
  OPTION_LIST_SUPPORTED_DEVICES,
  OPTION_LIST_SUPPORTED_DEVICES_XPAD,
  OPTION_LIST_CONTROLLER,
  OPTION_MOUSE,
  OPTION_EVDEV,
  OPTION_EVDEV_ABSMAP,
  OPTION_EVDEV_KEYMAP,
  OPTION_DETACH_KERNEL_DRIVER,
  OPTION_HELP_DEVICES
};

CommandLineParser::CommandLineParser() :
  m_argp(),
  m_ini()
{
  init_argp();
}

void
CommandLineParser::init_argp()
{
  m_argp
    .add_usage("[OPTION]...")
    .add_text("Xbox360 USB Gamepad Userspace Driver")
    .add_newline()
  
    .add_text("General Options: ")
    .add_option(OPTION_HELP,         'h', "help",         "", "display this help and exit")
    .add_option(OPTION_VERSION,      'V', "version",      "", "print the version number and exit")
    .add_option(OPTION_VERBOSE,      'v', "verbose",      "", "print verbose messages")
    .add_option(OPTION_HELP_LED,      0,  "help-led",     "", "list possible values for the led")
    .add_option(OPTION_HELP_DEVICES,  0,  "help-devices", "", "list supported devices")
    .add_option(OPTION_SILENT,       's', "silent",  "",  "do not display events on console")
    .add_option(OPTION_QUIET,         0,  "quiet",   "",  "do not display startup text")
    .add_option(OPTION_LIST_CONTROLLER, 'L', "list-controller", "", "list available controllers")
    .add_option(OPTION_LIST_SUPPORTED_DEVICES, 0, "list-supported-devices", "", "list supported devices (used by xboxdrv-daemon.py)")
    .add_option(OPTION_LIST_SUPPORTED_DEVICES_XPAD, 0, "list-supported-devices-xpad", "", "list supported devices in xpad.c style")
    .add_option(OPTION_TEST_RUMBLE,  'R', "test-rumble", "", "map rumbling to LT and RT (for testing only)")
    .add_option(OPTION_NO_UINPUT,     0,  "no-uinput",   "", "do not try to start uinput event dispatching")
    .add_option(OPTION_NO_EXTRA_DEVICES, 0,  "no-extra-devices",  "", "Do not create keyboard and mouse devices, just use a single device")
    .add_option(OPTION_MIMIC_XPAD,    0,  "mimic-xpad",  "", "Causes xboxdrv to use the same axis and button names as the xpad kernel driver")
    .add_option(OPTION_DAEMON,       'D', "daemon",      "", "run as daemon")
    .add_option(OPTION_CONFIG,       'c', "config",      "FILE", "read configuration from FILE")
    .add_option(OPTION_WRITE_CONFIG,  0, "write-config",      "FILE", "write an example configuration to FILE")
    .add_newline()

    .add_text("Device Options: ")
    .add_option(OPTION_DETACH_KERNEL_DRIVER, 'd', "detach-kernel-driver", "", "Detaches the kernel driver currently associated with the device")
    .add_option(OPTION_ID,           'i', "id",      "N", "use controller with id N (default: 0)")
    .add_option(OPTION_WID,          'w', "wid",     "N", "use wireless controller with wid N (default: 0)")
    .add_option(OPTION_DEVICE_BY_PATH, 0, "device-by-path", "BUS:DEV", "Use device BUS:DEV, do not do any scanning")
    .add_option(OPTION_DEVICE_BY_ID,   0, "device-by-id",   "VENDOR:PRODUCT", "Use device that matches VENDOR:PRODUCT (as returned by lsusb)")
    .add_option(OPTION_TYPE,           0, "type",    "TYPE", "Ignore autodetection and enforce controller type (xbox, xbox-mat, xbox360, xbox360-wireless, xbox360-guitar)")
    .add_option(OPTION_EVDEV,          0, "evdev",   "DEVICE", "Read events from a evdev device, instead of USB")
    .add_option(OPTION_EVDEV_ABSMAP, 0, "evdev-absmap", "MAP", "Map evdev key events to Xbox360 button events")
    .add_option(OPTION_EVDEV_KEYMAP, 0, "evdev-keymap", "MAP", "Map evdev abs events to Xbox360 axis events")
    .add_newline()

    .add_text("Status Options: ")
    .add_option(OPTION_LED,     'l', "led",    "NUM", "set LED status, see --help-led (default: 0)")
    .add_option(OPTION_RUMBLE,  'r', "rumble", "L,R", "set the speed for both rumble motors [0-255] (default: 0,0)")
    .add_option(OPTION_QUIT,    'q', "quit",   "",    "only set led and rumble status then quit")
    .add_newline()

    .add_text("Configuration Options: ")
    .add_option(OPTION_DEADZONE,           0, "deadzone",         "INT",  "Threshold under which axis events are ignored (default: 0)")
    .add_option(OPTION_DEADZONE_TRIGGER,   0, "deadzone-trigger", "INT",  "Threshold under which trigger events are ignored (default: 0)")
    .add_option(OPTION_TRIGGER_AS_BUTTON,  0, "trigger-as-button", "",    "LT and RT send button instead of axis events")
    .add_option(OPTION_TRIGGER_AS_ZAXIS,   0, "trigger-as-zaxis", "",     "Combine LT and RT to form a zaxis instead")
    .add_option(OPTION_DPAD_AS_BUTTON,     0, "dpad-as-button",   "",     "DPad sends button instead of axis events")
    .add_option(OPTION_DPAD_ONLY,          0, "dpad-only",        "",     "Both sticks are ignored, only DPad sends out axis events")
    .add_option(OPTION_BUTTONMAP,         'b', "buttonmap",      "MAP",   "Remap the buttons as specified by MAP (example: B=A,X=A,Y=A)")
    .add_option(OPTION_AXISMAP,           'a', "axismap",        "MAP",   "Remap the axis as specified by MAP (example: -Y1=Y1,X1=X2)")
    .add_option(OPTION_NAME,               0, "name",             "DEVNAME", "Changes the descriptive name the device will have")
    .add_option(OPTION_UI_CLEAR,           0, "ui-clear",         "",     "Removes all existing uinput bindings")
    .add_option(OPTION_UI_BUTTONMAP,       0, "ui-buttonmap",     "MAP",  "Changes the uinput events send when hitting a button (example: X=BTN_Y,A=KEY_A)")
    .add_option(OPTION_UI_AXISMAP,         0, "ui-axismap",       "MAP",  "Changes the uinput events send when moving a axis (example: X1=ABS_X2)")
    .add_option(OPTION_MOUSE,            'm', "mouse",            "",     "Enable mouse emulation")
    .add_option(OPTION_SQUARE_AXIS,        0, "square-axis",      "",     "Cause the diagonals to be reported as (1,1) instead of (0.7, 0.7)")
    .add_option(OPTION_FOUR_WAY_RESTRICTOR,0, "four-way-restrictor", "",  "Restrict axis movement to one axis at a time")
    .add_option(OPTION_DPAD_ROTATION,      0, "dpad-rotation",    "DEGREE", "Rotate the dpad by the given DEGREE, must be a multiple of 45")
    .add_option(OPTION_AXIS_SENSITIVITY,   0, "axis-sensitivity", "MAP",  "Adjust the axis sensitivity (example: X1=2.0,Y1=1.0)")
    .add_option(OPTION_RELATIVE_AXIS,      0, "relative-axis",    "MAP",  "Make an axis emulate a joystick throttle (example: y2=64000)")
    .add_option(OPTION_AUTOFIRE,           0, "autofire",         "MAP",  "Cause the given buttons to act as autofire (example: A=250)")
    .add_option(OPTION_CALIBRARIOTION,     0, "calibration",      "MAP",  "Changes the calibration for the given axis (example: X2=-32768:0:32767)")

    .add_text("Force Feedback: ")
    .add_option(OPTION_FORCE_FEEDBACK,     0, "force-feedback",   "",     "Enable force feedback support")
    .add_option(OPTION_RUMBLE_GAIN,        0, "rumble-gain",      "NUM",  "Set relative rumble strength (default: 255)")
    .add_newline()

    .add_text("See README for more documentation and examples.")
    .add_text("Report bugs to Ingo Ruhnke <grumbel@gmx.de>"); 
}

void
CommandLineParser::init_ini(Options* opts)
{
  m_ini.clear();

  m_ini.section("xboxdrv")
    ("verbose", &opts->verbose)
    ("silent", &opts->silent)
    ("quiet",  &opts->quiet)
    ("rumble", &opts->rumble)
    ("led", &opts->led)
    ("rumble-l", &opts->rumble_l)
    ("rumble-r", &opts->rumble_r)
    ("rumble-gain", &opts->rumble_gain)
    ("controller-id", &opts->controller_id)
    ("wireless-id", &opts->wireless_id)
    ("instant-exit", &opts->instant_exit)
    ("no-uinput", &opts->no_uinput)
    ("detach-kernel-driver", &opts->detach_kernel_driver)
    ("busid", &opts->busid)
    ("devid", &opts->devid)
    ("deadzone", &opts->deadzone)
    ("deadzone-trigger", &opts->deadzone_trigger)
    ("vendor-id", &opts->vendor_id)
    ("product-id", &opts->product_id)   
    ("square-axis", &opts->square_axis)
    ("four-way-restrictor", &opts->four_way_restrictor)
    ("dpad-rotation", &opts->dpad_rotation)
    ("evdev-device", &opts->evdev_device)

    // uinput stuff
    ("device-name", &opts->uinput_config.device_name)
    ("trigger-as-button", &opts->uinput_config.trigger_as_button)
    ("trigger-as-zaxis", &opts->uinput_config.trigger_as_zaxis)
    ("dpad-as-button", &opts->uinput_config.dpad_as_button)
    ("dpad-only", &opts->uinput_config.dpad_only)
    ("force-feedback", &opts->uinput_config.force_feedback)
    ("extra-devices", &opts->uinput_config.extra_devices)
    // FIXME: mimic_xpad()
    ;

  m_ini.section("ui-buttonmap", boost::bind(&CommandLineParser::set_ui_buttonmap, this, _1, _2));
  m_ini.section("ui-axismap",   boost::bind(&CommandLineParser::set_ui_axismap, this, _1, _2));

  m_ini.section("buttonmap", boost::bind(&CommandLineParser::set_buttonmap, this, _1, _2));
  m_ini.section("axismap",   boost::bind(&CommandLineParser::set_axismap, this, _1, _2));

  m_ini.section("autofire",   boost::bind(&CommandLineParser::set_autofire, this, _1, _2));
  m_ini.section("relative-axis",   boost::bind(&CommandLineParser::set_relative_axis, this, _1, _2));
  m_ini.section("calibration",   boost::bind(&CommandLineParser::set_calibration, this, _1, _2));
  m_ini.section("axis-sensitivity",   boost::bind(&CommandLineParser::set_calibration, this, _1, _2));

  m_ini.section("evdev-absmap", boost::bind(&CommandLineParser::set_evdev_absmap, this, _1, _2));
  m_ini.section("evdev-keymap", boost::bind(&CommandLineParser::set_evdev_keymap, this, _1, _2));
}

void set_ui_button_map(ButtonMap& ui_button_map, const std::string& str)
{
  std::string::size_type i = str.find('=');
  if (i == std::string::npos)
  {
    throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-button-mapping, '=' missing");
  }
  else
  {
    std::string btn_str = str.substr(0, i);
    ButtonEvent event = ButtonEvent::from_string(str.substr(i+1, str.size()-i));

    std::string::size_type j = btn_str.find('+');
    if (j == std::string::npos)
    {
      XboxButton  btn = string2btn(btn_str);

      ui_button_map.bind(btn, event);
    }
    else
    {
      XboxButton shift = string2btn(btn_str.substr(0, j));
      XboxButton btn   = string2btn(btn_str.substr(j+1));

      ui_button_map.bind(shift, btn, event);
    }
  }
}

void
CommandLineParser::set_ui_axismap_from_string(const std::string& str)
{
  std::string::size_type i = str.find_first_of('=');
  if (i == std::string::npos)
  {
    throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-axis-mapping");
  }
  else
  {
    set_ui_axismap(str.substr(0, i),
                   str.substr(i+1, str.size()-i));
  }
}

void set_evdev_absmap(std::map<int, XboxAxis>& absmap, const std::string& str)
{
  std::string lhs, rhs;
  split_string_at(str, '=', &lhs, &rhs);
  absmap[str2abs(lhs)] = string2axis(rhs);
}

void set_evdev_keymap(std::map<int, XboxButton>& keymap, const std::string& str)
{
  std::string lhs, rhs;
  split_string_at(str, '=', &lhs, &rhs);
  keymap[str2key(lhs)] = string2btn(rhs);
  std::cout << "KEY: " << str2key(lhs) << std::endl;
}

template<class C, class Func>
void arg2vector2(const std::string& str, typename std::vector<C>& lst, Func func)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
  {
    if (*i == ',')
    {
      if (i != start)
      {
        std::string lhs, rhs;
        split_string_at(std::string(start, i), '=', &lhs, &rhs);
        lst.push_back(func(lhs, rhs));
      }
          
      start = i+1;
    }
  }
  
  if (start != str.end())
  {
    std::string lhs, rhs;
    split_string_at(std::string(start, str.end()), '=', &lhs, &rhs);
    lst.push_back(func(lhs, rhs));
  }
}

void
CommandLineParser::parse_args(int argc, char** argv, Options* options)
{  
  init_ini(options);
  m_options = options;

  ArgParser::ParsedOptions parsed = m_argp.parse_args(argc, argv);

  for(ArgParser::ParsedOptions::const_iterator i = parsed.begin(); i != parsed.end(); ++i)
  {
    Options& opts = *options;
    const ArgParser::ParsedOption& opt = *i;

    switch (i->key)
    {
      case OPTION_HELP:
        opts.mode = Options::PRINT_HELP;
        break;

      case OPTION_VERSION:
        opts.mode = Options::PRINT_VERSION;
        break;
          
      case OPTION_VERBOSE:
        opts.verbose = true;
        break;

      case OPTION_QUIET:
        opts.quiet   = true;
        break;

      case OPTION_SILENT:
        opts.silent = true;
        break;

      case OPTION_DAEMON:
        opts.silent = true;
        opts.mode = Options::RUN_DAEMON;
        break;

      case OPTION_WRITE_CONFIG:
        {
          opts.instant_exit = true;

          std::ofstream out(opt.argument.c_str());
          if (!out)
          {
            std::ostringstream str;
            str << "Couldn't create " << opt.argument;
            throw std::runtime_error(str.str());
          }
          else
          {
            // FIXME: implement me
            m_ini.save(out);
          }
        }
      break;

      case OPTION_CONFIG:
        {
          std::ifstream in(opt.argument.c_str());
          if (!in)
          {
            std::ostringstream str;
            str << "Couldn't open " << opt.argument;
            throw std::runtime_error(str.str());
          }
          else
          {
            INISchemaBuilder builder(m_ini);
            INIParser parser(in, builder, opt.argument);
            parser.run();
          }
        }
        break;

      case OPTION_TEST_RUMBLE:
        opts.rumble = true;
        break;

      case OPTION_RUMBLE:
        if (sscanf(opt.argument.c_str(), "%d,%d", &opts.rumble_l, &opts.rumble_r) == 2)
        {
          opts.rumble_l = std::max(0, std::min(255, opts.rumble_l));
          opts.rumble_r = std::max(0, std::min(255, opts.rumble_r));
        }
        else
        {
          RAISE_EXCEPTION(opt.option << " expected an argument in form INT,INT");
        }
        break;

      case OPTION_QUIT:
        opts.instant_exit = true;
        break;

      case OPTION_NO_UINPUT:
        opts.no_uinput = true;
        break;

      case OPTION_MIMIC_XPAD:
        opts.uinput_config.mimic_xpad();
        break;

      case OPTION_NO_EXTRA_DEVICES:
        opts.uinput_config.extra_devices = false;
        break;

      case OPTION_TYPE:
        if (opt.argument == "xbox")
        {
          opts.gamepad_type = GAMEPAD_XBOX;
        }
        else if (opt.argument == "xbox-mat")
        {
          opts.gamepad_type = GAMEPAD_XBOX_MAT;
        }
        else if (opt.argument == "xbox360")
        {
          opts.gamepad_type = GAMEPAD_XBOX360;
        }
        else if (opt.argument == "xbox360-guitar")
        {
          opts.gamepad_type = GAMEPAD_XBOX360_GUITAR;
        }
        else if (opt.argument == "xbox360-wireless")
        {
          opts.gamepad_type = GAMEPAD_XBOX360_WIRELESS;
        }
        else if (opt.argument == "firestorm")
        {
          opts.gamepad_type = GAMEPAD_FIRESTORM;
        }
        else if (opt.argument == "firestorm-vsb")
        {
          opts.gamepad_type = GAMEPAD_FIRESTORM_VSB;
        }
        else if (opt.argument == "saitek-p2500")
        {
          opts.gamepad_type = GAMEPAD_SAITEK_P2500;
        }
        else
        {
          RAISE_EXCEPTION("unknown type: " << opt.argument << '\n'
                          << "Possible types are:\n"
                          << " * xbox\n"
                          << " * xbox-mat\n"
                          << " * xbox360\n"
                          << " * xbox360-guitar\n"
                          << " * xbox360-wireless\n"
                          << " * firestorm\n"
                          << " * firestorm-vsb\n"
                          << " * saitek-p2500\n");
        }
        break;

      case OPTION_FORCE_FEEDBACK:
        opts.uinput_config.force_feedback = true;
        break;

      case OPTION_RUMBLE_GAIN:
        opts.rumble_gain = to_number(255, opt.argument);
        break;

      case OPTION_BUTTONMAP:
        arg2vector2(opt.argument, opts.button_map, &ButtonMapping::from_string);
        break;

      case OPTION_AXISMAP:
        arg2vector2(opt.argument, opts.axis_map, &AxisMapping::from_string);
        break;
                    
      case OPTION_NAME:
        opts.uinput_config.device_name = opt.argument;
        break;

      case OPTION_UI_CLEAR:
        std::fill_n(opts.uinput_config.axis_map, static_cast<int>(XBOX_AXIS_MAX), AxisEvent::invalid());
        opts.uinput_config.btn_map.clear();
        break;

      case OPTION_UI_AXISMAP:
        arg2apply(opt.argument, boost::bind(&CommandLineParser::set_ui_axismap_from_string, this, _1));
        break;

      case OPTION_UI_BUTTONMAP:
        arg2apply(opt.argument, boost::bind(&set_ui_button_map, boost::ref(opts.uinput_config.btn_map), _1));
        break;

      case OPTION_MOUSE:
        opts.uinput_config.dpad_as_button = true;
        opts.deadzone = 4000;
        opts.uinput_config.trigger_as_zaxis = true;
        arg2vector2("-y2=y2,-trigger=trigger", opts.axis_map, &AxisMapping::from_string);
        // send events only every 20msec, lower values cause a jumpy pointer
        arg2apply("x1=REL_X:15:20,y1=REL_Y:15:20,"
                  "y2=REL_WHEEL:5:100,x2=REL_HWHEEL:5:100,"
                  "trigger=REL_WHEEL:5:100",
                  boost::bind(&CommandLineParser::set_ui_axismap_from_string, this, _1));
        arg2apply("a=BTN_LEFT,b=BTN_RIGHT,x=BTN_MIDDLE,y=KEY_ENTER,rb=KEY_PAGEDOWN,lb=KEY_PAGEUP,"
                  "dl=KEY_LEFT,dr=KEY_RIGHT,du=KEY_UP,dd=KEY_DOWN,"
                  "start=KEY_FORWARD,back=KEY_BACK,guide=KEY_ESC,"
                  "tl=void,tr=void",
                  boost::bind(&set_ui_button_map, boost::ref(opts.uinput_config.btn_map), _1));
        break;

      case OPTION_DETACH_KERNEL_DRIVER:
        opts.detach_kernel_driver = true;
        break;

      case OPTION_EVDEV:
        opts.evdev_device = opt.argument;
        break;
        
      case OPTION_EVDEV_ABSMAP:
        arg2apply(opt.argument, boost::bind(&::set_evdev_absmap, boost::ref(opts.evdev_absmap), _1));
        break;

      case OPTION_EVDEV_KEYMAP:
        arg2apply(opt.argument, boost::bind(&::set_evdev_keymap, boost::ref(opts.evdev_keymap), _1));
        break;

      case OPTION_ID:
        opts.controller_id = boost::lexical_cast<int>(opt.argument);
        break;

      case OPTION_WID:
        opts.wireless_id = boost::lexical_cast<int>(opt.argument);
        if (opts.wireless_id < 0 || opts.wireless_id > 3)
        {
          throw std::runtime_error("wireless id must be within 0 and 3");
        }
        break;

      case OPTION_LED:
        if (opt.argument == "help")
        {
          opts.mode = Options::PRINT_LED_HELP;
        }
        else
        {
          opts.led = boost::lexical_cast<int>(opt.argument);
        }
        break;
            
      case OPTION_DPAD_ONLY:
        if (opts.uinput_config.dpad_as_button)
          RAISE_EXCEPTION("Can't combine --dpad-as-button with --dpad-only");
            
        opts.uinput_config.dpad_only = true;
        break;
            
      case OPTION_DPAD_AS_BUTTON:
        if (opts.uinput_config.dpad_only)
          throw std::runtime_error("Can't combine --dpad-as-button with --dpad-only");

        opts.uinput_config.dpad_as_button = true;
        break;

      case OPTION_DEADZONE:
        opts.deadzone = to_number(32767, opt.argument);
        break;

      case OPTION_DEADZONE_TRIGGER:
        opts.deadzone_trigger = to_number(255, opt.argument);
        break;

      case OPTION_TRIGGER_AS_BUTTON:
        if (opts.uinput_config.trigger_as_zaxis)
        {
          RAISE_EXCEPTION("Can't combine --trigger-as-button and --trigger-as-zaxis");
        }
        else
        {
          opts.uinput_config.trigger_as_button = true;
        }
        break;

      case OPTION_AUTOFIRE:
        arg2vector2(opt.argument, opts.autofire_map, &AutoFireMapping::from_string);
        break;

      case OPTION_CALIBRARIOTION:
        arg2vector2(opt.argument, opts.calibration_map, &CalibrationMapping::from_string);
        break;

      case OPTION_RELATIVE_AXIS:
        arg2vector2(opt.argument, opts.relative_axis_map, &RelativeAxisMapping::from_string);
        break;

      case OPTION_AXIS_SENSITIVITY:
        arg2vector2(opt.argument, opts.axis_sensitivity_map, &AxisSensitivityMapping::from_string);
        break;

      case OPTION_FOUR_WAY_RESTRICTOR:
        opts.four_way_restrictor = true;
        break;

      case OPTION_DPAD_ROTATION:
        {
          int degree = boost::lexical_cast<int>(opt.argument);
          degree /= 45;
          degree %= 8;
          if (degree < 0) degree += 8;
          opts.dpad_rotation = degree;
        }
        break;

      case OPTION_SQUARE_AXIS:
        opts.square_axis = true;
        break;

      case OPTION_TRIGGER_AS_ZAXIS:
        if (opts.uinput_config.trigger_as_button)
        {
          RAISE_EXCEPTION("Can't combine --trigger-as-button and --trigger-as-zaxis");
        }
        else
        {
          opts.uinput_config.trigger_as_zaxis = true;
        }
        break;

      case OPTION_HELP_LED:
        opts.mode = Options::PRINT_LED_HELP;
        break;

      case OPTION_DEVICE_BY_ID:
        {
          unsigned int tmp_product_id;
          unsigned int tmp_vendor_id;
          if (sscanf(opt.argument.c_str(), "%x:%x", &tmp_vendor_id, &tmp_product_id) == 2)
          {
            opts.vendor_id  = tmp_vendor_id;
            opts.product_id = tmp_product_id;
          }
          else
          {
            RAISE_EXCEPTION(opt.option << " expected an argument in form PRODUCT:VENDOR (i.e. 046d:c626)");
          }
          break;
        }

      case OPTION_DEVICE_BY_PATH:
        {
          char busid[4] = { '\0' };
          char devid[4] = { '\0' };

          if (sscanf(opt.argument.c_str(), "%3s:%3s", busid, devid) != 2)
          {  
            RAISE_EXCEPTION(opt.option << " expected an argument in form BUS:DEV (i.e. 006:003)");
          }
          else
          {
            opts.busid = busid;
            opts.devid = devid;
          }
        }
        break;

      case OPTION_LIST_SUPPORTED_DEVICES:
        opts.mode = Options::RUN_LIST_SUPPORTED_DEVICES;
        break;

      case OPTION_LIST_SUPPORTED_DEVICES_XPAD:
        opts.mode = Options::RUN_LIST_SUPPORTED_DEVICES_XPAD;
        break;

      case OPTION_LIST_CONTROLLER:
        opts.mode = Options::RUN_LIST_CONTROLLER;
        break;

      case OPTION_HELP_DEVICES:
        opts.mode = Options::PRINT_HELP_DEVICES;
        break;

      case ArgParser::REST_ARG:
        RAISE_EXCEPTION("unknown command line option: " << opt.argument);
        break;

      default:
        RAISE_EXCEPTION("unknown command line option: " << opt.option);
        break;
    }
  }
}

void
CommandLineParser::print_help() const
{
  m_argp.print_help(std::cout);
}

void
CommandLineParser::print_led_help() const
{
  std::cout << 
    "Possible values for '--led VALUE' are:\n\n"
    "   0: off\n"
    "   1: all blinking\n"
    "   2: 1/top-left blink, then on\n"
    "   3: 2/top-right blink, then on\n"
    "   4: 3/bottom-left blink, then on\n"
    "   5: 4/bottom-right blink, then on\n"
    "   6: 1/top-left on\n"
    "   7: 2/top-right on\n"
    "   8: 3/bottom-left on\n"
    "   9: 4/bottom-right on\n"
    "  10: rotate\n"
    "  11: blink\n"
    "  12: blink slower\n"
    "  13: rotate with two lights\n"
    "  14: blink\n"
    "  15: blink once\n"
            << std::endl;
}
  
void
CommandLineParser::print_version() const
{
  std::cout
    << "xboxdrv " PACKAGE_VERSION " - http://pingus.seul.org/~grumbel/xboxdrv/\n"
    << "Copyright © 2008-2010 Ingo Ruhnke <grumbel@gmx.de>\n"
    << "Licensed under GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
    << "This program comes with ABSOLUTELY NO WARRANTY.\n"
    << "This is free software, and you are welcome to redistribute it under certain conditions; see the file COPYING for details.\n";
}

void
CommandLineParser::set_ui_axismap(const std::string& name, const std::string& value)
{
  XboxAxis  axis  = string2axis(name);
  AxisEvent event = AxisEvent::from_string(value);
  if (axis != XBOX_AXIS_UNKNOWN)
  {
    m_options->uinput_config.axis_map[axis] = event;
  }
  else
  {
    throw std::runtime_error("Couldn't convert string \"" + name + "=" + value + "\" to ui-axis-mapping");
  }      
}

void
CommandLineParser::set_ui_buttonmap(const std::string& name, const std::string& value)
{
  std::string btn_str = name;
  ButtonEvent event = ButtonEvent::from_string(value);

  std::string::size_type j = btn_str.find('+');
  if (j == std::string::npos)
  {
    XboxButton  btn = string2btn(btn_str);

    m_options->uinput_config.btn_map.bind(btn, event);
  }
  else
  {
    XboxButton shift = string2btn(btn_str.substr(0, j));
    XboxButton btn   = string2btn(btn_str.substr(j+1));

    m_options->uinput_config.btn_map.bind(shift, btn, event);
  }
}

void
CommandLineParser::set_axismap(const std::string& name, const std::string& value)
{
  m_options->axis_map.push_back(AxisMapping::from_string(name, value));
}

void
CommandLineParser::set_buttonmap(const std::string& name, const std::string& value)
{
  m_options->button_map.push_back(ButtonMapping::from_string(name, value));
}

void
CommandLineParser::set_evdev_absmap(const std::string& name, const std::string& value)
{
  m_options->evdev_absmap[str2abs(name)] = string2axis(value);
}

void
CommandLineParser::set_evdev_keymap(const std::string& name, const std::string& value)
{
  m_options->evdev_keymap[str2key(name)] = string2btn(value);
}

void
CommandLineParser::set_relative_axis(const std::string& name, const std::string& value)
{
  m_options->relative_axis_map.push_back(RelativeAxisMapping::from_string(name, value));
}

void
CommandLineParser::set_autofire(const std::string& name, const std::string& value)
{
  m_options->autofire_map.push_back(AutoFireMapping::from_string(name, value));
}

void
CommandLineParser::set_calibration(const std::string& name, const std::string& value)
{
  m_options->calibration_map.push_back(CalibrationMapping::from_string(name, value));
}

void
CommandLineParser::set_axis_sensitivity(const std::string& name, const std::string& value)
{
  m_options->axis_sensitivity_map.push_back(AxisSensitivityMapping::from_string(name, value));
}

/* EOF */
