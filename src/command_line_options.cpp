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

#include <fstream>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

#include "helper.hpp"
#include "ini_parser.hpp"
#include "ini_schema_builder.hpp"
#include "options.hpp"

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
  OPTION_CONFIG_OPTION,
  OPTION_CONFIG,
  OPTION_ALT_CONFIG,
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
  OPTION_MODIFIER,
  OPTION_BUTTONMAP,
  OPTION_AXISMAP,
  OPTION_NAME,
  OPTION_NEXT,
  OPTION_UI_CLEAR,
  OPTION_TOGGLE,
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
  OPTION_GUITAR,
  OPTION_EVDEV,
  OPTION_EVDEV_NO_GRAB,
  OPTION_EVDEV_DEBUG,
  OPTION_EVDEV_ABSMAP,
  OPTION_EVDEV_KEYMAP,
  OPTION_CHATPAD,
  OPTION_CHATPAD_NO_INIT,
  OPTION_CHATPAD_DEBUG,
  OPTION_TIMEOUT,
  OPTION_HEADSET,
  OPTION_HEADSET_DUMP,
  OPTION_HEADSET_PLAY,
  OPTION_DETACH_KERNEL_DRIVER,
  OPTION_DAEMON_DETACH,
  OPTION_DAEMON_PID_FILE,
  OPTION_HELP_DEVICES,
  OPTION_LIST_ALL,
  OPTION_LIST_ABS,
  OPTION_LIST_REL,
  OPTION_LIST_KEY,
  OPTION_LIST_X11KEYSYM,
  OPTION_LIST_AXIS,
  OPTION_LIST_BUTTON,
  OPTION_DAEMON_ON_CONNECT,
  OPTION_DAEMON_ON_DISCONNECT
};

CommandLineParser::CommandLineParser() :
  m_argp(),
  m_ini(),
  m_options()
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
    .add_option(OPTION_CONFIG,       'c', "config",      "FILE", "read configuration from FILE")
    .add_option(OPTION_ALT_CONFIG,    0, "alt-config",   "FILE", "read alternative configuration from FILE ")
    .add_option(OPTION_CONFIG_OPTION,'o', "option",      "NAME=VALUE", "Set the given configuration option") 
    .add_option(OPTION_WRITE_CONFIG,  0, "write-config", "FILE", "rite an example configuration to FILE")
    .add_newline()

    .add_text("List Options: ")
    .add_option(OPTION_LIST_ABS,       0, "list-abs",       "", "List all possible EV_ABS names")
    .add_option(OPTION_LIST_REL,       0, "list-rel",       "", "List all possible EV_REL names")
    .add_option(OPTION_LIST_KEY,       0, "list-key",       "", "List all possible EV_KEY names")
    .add_option(OPTION_LIST_X11KEYSYM, 0, "list-x11keysym", "", "List all possible X11KeySym")
    .add_option(OPTION_LIST_AXIS,      0, "list-axis",      "", "List all possible XboxAxis")
    .add_option(OPTION_LIST_BUTTON,    0, "list-button",    "", "List all possible XboxButton")
    .add_option(OPTION_LIST_ALL,       0, "list-all",       "", "List all of the above")
    .add_newline()

    .add_text("Daemon Options: ")
    .add_option(OPTION_DAEMON,        'D', "daemon",    "", "Run as daemon")
    .add_option(OPTION_DAEMON_DETACH,   0, "detach",      "", "Detach the daemon from the current shell")
    .add_option(OPTION_DAEMON_PID_FILE, 0, "pid-file",    "FILE", "Write daemon pid to FILE")
    .add_option(OPTION_DAEMON_ON_CONNECT,    0, "on-connect", "FILE", "Launch EXE when a new controller is connected")
    .add_option(OPTION_DAEMON_ON_DISCONNECT, 0, "on-disconnect", "FILE", "Launch EXE when a controller is disconnected")
    .add_newline()

    .add_text("Device Options: ")
    .add_option(OPTION_DETACH_KERNEL_DRIVER, 'd', "detach-kernel-driver", "", "Detaches the kernel driver currently associated with the device")
    .add_option(OPTION_ID,           'i', "id",      "N", "use controller with id N (default: 0)")
    .add_option(OPTION_WID,          'w', "wid",     "N", "use wireless controller with wid N (default: 0)")
    .add_option(OPTION_DEVICE_BY_PATH, 0, "device-by-path", "BUS:DEV", "Use device BUS:DEV, do not do any scanning")
    .add_option(OPTION_DEVICE_BY_ID,   0, "device-by-id",   "VENDOR:PRODUCT", "Use device that matches VENDOR:PRODUCT (as returned by lsusb)")
    .add_option(OPTION_TYPE,           0, "type",    "TYPE", "Ignore autodetection and enforce controller type (xbox, xbox-mat, xbox360, xbox360-wireless, xbox360-guitar)")
    .add_newline()

    .add_text("Evdev Options: ")
    .add_option(OPTION_EVDEV,          0, "evdev",   "DEVICE", "Read events from a evdev device, instead of USB")
    .add_option(OPTION_EVDEV_DEBUG,    0, "evdev-debug", "", "Print out all events received from evdev")
    .add_option(OPTION_EVDEV_NO_GRAB,  0, "evdev-no-grab", "", "Do not grab the event device, allow other apps to receive events")
    .add_option(OPTION_EVDEV_ABSMAP,   0, "evdev-absmap", "MAP", "Map evdev key events to Xbox360 button events")
    .add_option(OPTION_EVDEV_KEYMAP,   0, "evdev-keymap", "MAP", "Map evdev abs events to Xbox360 axis events")
    .add_newline()

    .add_text("Status Options: ")
    .add_option(OPTION_LED,     'l', "led",    "NUM", "set LED status, see --help-led (default: 0)")
    .add_option(OPTION_QUIT,    'q', "quit",   "",    "only set led and rumble status then quit")
    .add_newline()

    .add_text("Chatpad Options (experimental): ")
    .add_option(OPTION_CHATPAD,        0, "chatpad", "",  "Enable Chatpad support for Xbox360 USB controller")
    .add_option(OPTION_CHATPAD_NO_INIT, 0, "chatpad-no-init", "",  "To not send init code to the Chatpad")
    .add_option(OPTION_CHATPAD_DEBUG, 0, "chatpad-debug", "",  "To not send init code to the Chatpad")
    .add_newline()

    .add_text("Headset Options (experimental, Xbox360 USB only): ")
    .add_option(OPTION_HEADSET,        0, "headset", "",  "Enable Headset support for Xbox360 USB controller (not working)")
    .add_option(OPTION_HEADSET_DUMP,   0, "headset-dump", "FILE",  "Dump headset data to FILE")
    .add_option(OPTION_HEADSET_PLAY,   0, "headset-play", "FILE",  "Play FILE on the headset")
    .add_newline()

    .add_text("Force Feedback: ")
    .add_option(OPTION_FORCE_FEEDBACK,     0, "force-feedback",   "",     "Enable force feedback support")
    .add_option(OPTION_RUMBLE_GAIN,        0, "rumble-gain",      "NUM",  "Set relative rumble strength (default: 255)")
    .add_option(OPTION_TEST_RUMBLE,      'R', "test-rumble", "", "map rumbling to LT and RT (for testing only)")
    .add_option(OPTION_RUMBLE,           'r', "rumble", "L,R", "set the speed for both rumble motors [0-255] (default: 0,0)")
    .add_newline()

    .add_text("Configuration Options: ")
    .add_option(OPTION_NEXT,               0, "next",   "", "Create a new configuration entry", false)
    .add_option(OPTION_NEXT,               0, "ui-new", "", "", false) // backward compatibility

    .add_option(OPTION_TOGGLE,             0, "toggle", "BTN", "Set button to use for toggling between configs")
    .add_option(OPTION_TOGGLE,             0, "ui-toggle", "BTN", "") // backward compatibility

    .add_option(OPTION_MODIFIER,           0, "modifier",        "MOD=ARGS", "Add a modifier to the modifier spec")
    .add_option(OPTION_TIMEOUT,            0, "timeout",         "INT",  "Amount of time to wait fo a device event before processing autofire, etc. (default: 25)")
    
    .add_option(OPTION_DEADZONE,           0, "deadzone",         "INT",  "Threshold under which axis events are ignored (default: 0)")
    .add_option(OPTION_DEADZONE_TRIGGER,   0, "deadzone-trigger", "INT",  "Threshold under which trigger events are ignored (default: 0)")
    .add_option(OPTION_BUTTONMAP,         'b', "buttonmap",      "MAP",   "Remap the buttons as specified by MAP (example: B=A,X=A,Y=A)")
    .add_option(OPTION_AXISMAP,           'a', "axismap",        "MAP",   "Remap the axis as specified by MAP (example: -Y1=Y1,X1=X2)")
    .add_option(OPTION_SQUARE_AXIS,        0, "square-axis",       "",     "Cause the diagonals to be reported as (1,1) instead of (0.7, 0.7)")
    .add_option(OPTION_FOUR_WAY_RESTRICTOR,0, "four-way-restrictor", "",  "Restrict axis movement to one axis at a time")
    .add_option(OPTION_DPAD_ROTATION,      0, "dpad-rotation",    "DEGREE", "Rotate the dpad by the given DEGREE, must be a multiple of 45")
    .add_option(OPTION_AXIS_SENSITIVITY,   0, "axis-sensitivity", "MAP",  "Adjust the axis sensitivity (example: X1=2.0,Y1=1.0)")
    .add_option(OPTION_RELATIVE_AXIS,      0, "relative-axis",    "MAP",  "Make an axis emulate a joystick throttle (example: y2=64000)")
    .add_option(OPTION_AUTOFIRE,           0, "autofire",         "MAP",  "Cause the given buttons to act as autofire (example: A=250)")
    .add_option(OPTION_CALIBRARIOTION,     0, "calibration",      "MAP",  "Changes the calibration for the given axis (example: X2=-32768:0:32767)")
    .add_newline()

    .add_text("Uinput Preset Configuration Options: ")
    .add_option(OPTION_TRIGGER_AS_BUTTON,  0, "trigger-as-button", "",    "LT and RT send button instead of axis events")
    .add_option(OPTION_TRIGGER_AS_ZAXIS,   0, "trigger-as-zaxis", "",     "Combine LT and RT to form a zaxis instead")
    .add_option(OPTION_DPAD_AS_BUTTON,     0, "dpad-as-button",   "",     "DPad sends button instead of axis events")
    .add_option(OPTION_DPAD_ONLY,          0, "dpad-only",        "",     "Both sticks are ignored, only DPad sends out axis events")
    .add_option(OPTION_MOUSE,             'm', "mouse",            "",     "Enable mouse emulation")
    .add_option(OPTION_GUITAR,             0, "guitar",            "",     "Enables guitar button and axis mapping")
    .add_option(OPTION_MIMIC_XPAD,         0,  "mimic-xpad",  "", "Causes xboxdrv to use the same axis and button names as the xpad kernel driver")
    .add_newline()

    .add_text("Uinput Configuration Options: ")
    .add_option(OPTION_NO_UINPUT,          0, "no-uinput",   "", "do not try to start uinput event dispatching")
    .add_option(OPTION_NAME,               0, "name",             "DEVNAME", "Changes the descriptive name the device will have")
    .add_option(OPTION_UI_CLEAR,           0, "ui-clear",         "",     "Removes all existing uinput bindings")
    .add_option(OPTION_UI_BUTTONMAP,       0, "ui-buttonmap",     "MAP",  "Changes the uinput events send when hitting a button (example: X=BTN_Y,A=KEY_A)")
    .add_option(OPTION_UI_AXISMAP,         0, "ui-axismap",       "MAP",  "Changes the uinput events send when moving a axis (example: X1=ABS_X2)")
    .add_option(OPTION_NO_EXTRA_DEVICES,   0, "no-extra-devices",  "", "Do not create separate virtual keyboard and mouse devices, just use a single virtual device")
    .add_newline()

    .add_text("Axis Filter:")
    .add_pseudo("  cal, calibration MIN:CENTER:MAX", "Set the calibration values for the axis")
    .add_pseudo("  sen, sensitivity:SENSITIVITY", "Set the axis sensitivity")
    .add_pseudo("  dead:VALUE, dead:MIN:CENTER:MAX", "Set the axis deadzone")
    .add_pseudo("  rel, relative:SPEED", "Turn axis into a relative-axis")
    .add_pseudo("  resp, response:VALUES:...", "Set values of the response curve")
    .add_pseudo("  log:STRING", "Print axis value to stdout")
    .add_newline()

    .add_text("Button Filter:")
    .add_pseudo("  tog, toggle", "Turn button into a toggle button")
    .add_pseudo("  inv, invert", "Invert the button value")
    .add_pseudo("  auto, autofire:RATE:DELAY", "Enable automatic button press repetition")
    .add_pseudo("  log:STRING", "Print button value to stdout")
    .add_newline()
  
    .add_text("See README for more documentation and examples.")
    .add_text("Report bugs to Ingo Ruhnke <grumbel@gmail.com>");
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
    ("vendor-id", &opts->vendor_id)
    ("product-id", &opts->product_id)   
    ("evdev", &opts->evdev_device)
    ("evdev-grab", &opts->evdev_grab)
    ("evdev-debug", &opts->evdev_debug)
    ("config", boost::bind(&CommandLineParser::read_config_file, this, opts, _1))
    ("alt-config", boost::bind(&CommandLineParser::read_alt_config_file, this, opts, _1))
    ("timeout", &opts->timeout)

    ("deadzone", boost::bind(&CommandLineParser::set_deadzone, this, _1))
    ("deadzone-trigger", boost::bind(&CommandLineParser::set_deadzone_trigger, this, _1))
    ("square-axis", boost::bind(&CommandLineParser::set_square_axis, this), boost::function<void ()>())
    ("four-way-restrictor", boost::bind(&CommandLineParser::set_four_way_restrictor, this), boost::function<void ()>())
    ("dpad-rotation", boost::bind(&CommandLineParser::set_dpad_rotation, this, _1))

    // uinput stuff
    ("device-name", &opts->controller.back().uinput.device_name)
    ("mouse", boost::bind(&UInputOptions::mouse, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("guitar", boost::bind(&UInputOptions::guitar, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("trigger-as-button", boost::bind(&UInputOptions::trigger_as_button, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("trigger-as-zaxis", boost::bind(&UInputOptions::trigger_as_zaxis, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("dpad-as-button", boost::bind(&UInputOptions::dpad_as_button, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("dpad-only", boost::bind(&UInputOptions::dpad_only, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())
    ("force-feedback", &opts->controller.back().uinput.force_feedback)
    ("mimic-xpad", boost::bind(&UInputOptions::mimic_xpad, boost::ref(opts->controller.back().uinput)), boost::function<void ()>())

    ("chatpad",         &opts->chatpad)
    ("chatpad-no-init", &opts->chatpad_no_init)
    ("chatpad-debug",   &opts->chatpad_debug)

    ("headset",         &opts->headset)
    ("headset-debug",   &opts->headset_debug)
    ("headset-dump",    &opts->headset_dump)
    ("headset-play",    &opts->headset_play)
    ;

  m_ini.section("xboxdrv-daemon")
    ("detach",        &opts->detach)
    ("pid-file",        &opts->pid_file)
    ("on-connect",    &opts->on_connect)
    ("on-disconnect", &opts->on_disconnect)
    ;

  m_ini.section("modifier", boost::bind(&CommandLineParser::set_modifier, this, _1, _2));

  m_ini.section("ui-buttonmap", boost::bind(&UInputOptions::set_ui_buttonmap, 
                                            boost::ref(opts->controller.back().uinput), _1, _2));
  m_ini.section("ui-axismap",   boost::bind(&UInputOptions::set_ui_axismap, 
                                            boost::ref(opts->controller.back().uinput), _1, _2));

  m_ini.section("buttonmap", boost::bind(&CommandLineParser::set_buttonmap, this, _1, _2));
  m_ini.section("axismap",   boost::bind(&CommandLineParser::set_axismap, this, _1, _2));

  m_ini.section("autofire",   boost::bind(&CommandLineParser::set_autofire, this, _1, _2));
  m_ini.section("relative-axis",   boost::bind(&CommandLineParser::set_relative_axis, this, _1, _2));
  m_ini.section("calibration",   boost::bind(&CommandLineParser::set_calibration, this, _1, _2));
  m_ini.section("axis-sensitivity",   boost::bind(&CommandLineParser::set_axis_sensitivity, this, _1, _2));

  m_ini.section("evdev-absmap", boost::bind(&CommandLineParser::set_evdev_absmap, this, _1, _2));
  m_ini.section("evdev-keymap", boost::bind(&CommandLineParser::set_evdev_keymap, this, _1, _2));
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

      case OPTION_CONFIG_OPTION:
        {
          std::string name, value;
          split_string_at(opt.argument, '=', &name, &value);

          INISchemaBuilder builder(m_ini);
          builder.send_section("xboxdrv");
          builder.send_pair(name, value);
        }
        break;

      case OPTION_CONFIG:
        read_config_file(&opts, opt.argument);
        break;

      case OPTION_ALT_CONFIG:
        read_alt_config_file(&opts, opt.argument);
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

      case OPTION_TIMEOUT:
        opts.timeout = boost::lexical_cast<int>(opt.argument);
        break;

      case OPTION_NO_UINPUT:
        opts.no_uinput = true;
        break;

      case OPTION_MIMIC_XPAD:
        opts.controller.back().uinput.mimic_xpad();
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

      case OPTION_CHATPAD:
        opts.chatpad = true;
        break;

      case OPTION_CHATPAD_NO_INIT:
        opts.chatpad_no_init = true;
        opts.chatpad = true;
        break;

      case OPTION_CHATPAD_DEBUG:
        opts.chatpad_debug = true;
        break;

      case OPTION_HEADSET:
        opts.headset = true;
        break;

      case OPTION_HEADSET_DUMP:
        opts.headset = true;
        opts.headset_dump = opt.argument;
        break;

      case OPTION_HEADSET_PLAY:
        opts.headset = true;
        opts.headset_play = opt.argument;
        break;

      case OPTION_FORCE_FEEDBACK:
        opts.controller.back().uinput.force_feedback = true;
        break;

      case OPTION_RUMBLE_GAIN:
        opts.rumble_gain = to_number(255, opt.argument);
        break;

      case OPTION_MODIFIER:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_modifier, this, _1, _2));
        break;

      case OPTION_BUTTONMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_buttonmap, this, _1, _2));
        break;

      case OPTION_AXISMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_axismap, this, _1, _2));
        break;
                    
      case OPTION_NAME:
        opts.controller.back().uinput.device_name = opt.argument;
        break;

      case OPTION_NEXT:
        opts.controller.push_back(ControllerOptions());
        if (opts.config_toggle_button == XBOX_BTN_UNKNOWN)
        {
          opts.config_toggle_button = XBOX_BTN_GUIDE;
        }
        break;

      case OPTION_TOGGLE:
        opts.config_toggle_button = string2btn(opt.argument);
        break;

      case OPTION_UI_CLEAR:
        opts.controller.back().uinput.get_axis_map().clear();
        opts.controller.back().uinput.get_btn_map().clear();
        break;

      case OPTION_UI_AXISMAP:
        process_name_value_string(opt.argument, boost::bind(&UInputOptions::set_ui_axismap, 
                                                            boost::ref(opts.controller.back().uinput), _1, _2));
        break;

      case OPTION_UI_BUTTONMAP:
        process_name_value_string(opt.argument, boost::bind(&UInputOptions::set_ui_buttonmap, 
                                                            boost::ref(opts.controller.back().uinput), _1, _2));
        break;

      case OPTION_MOUSE:
        opts.controller.back().uinput.mouse();
        break;

      case OPTION_GUITAR:
        opts.controller.back().uinput.guitar();
        break;

      case OPTION_DETACH_KERNEL_DRIVER:
        opts.detach_kernel_driver = true;
        break;

      case OPTION_EVDEV:
        opts.evdev_device = opt.argument;
        break;

      case OPTION_EVDEV_DEBUG:
        opts.evdev_debug = true;
        break;
        
      case OPTION_EVDEV_NO_GRAB:
        opts.evdev_grab = false;
        break;

      case OPTION_EVDEV_ABSMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_evdev_absmap, this, _1, _2));
        break;

      case OPTION_EVDEV_KEYMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_evdev_keymap, this, _1, _2));
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
        opts.controller.back().uinput.dpad_only();
        break;
            
      case OPTION_DPAD_AS_BUTTON:
        opts.controller.back().uinput.dpad_as_button();
        break;

      case OPTION_DEADZONE:
        set_deadzone(opt.argument);
        break;

      case OPTION_DEADZONE_TRIGGER:
        set_deadzone_trigger(opt.argument);
        break;

      case OPTION_TRIGGER_AS_BUTTON:
        opts.controller.back().uinput.trigger_as_button();
        break;
        
      case OPTION_TRIGGER_AS_ZAXIS:
        opts.controller.back().uinput.trigger_as_zaxis();
        break;

      case OPTION_AUTOFIRE:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_autofire, this, _1, _2));
        break;

      case OPTION_CALIBRARIOTION:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_calibration, this, _1, _2));
        break;

      case OPTION_RELATIVE_AXIS:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_relative_axis, this, _1, _2));
        break;

      case OPTION_AXIS_SENSITIVITY:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_axis_sensitivity, this, _1, _2));
        break;

      case OPTION_FOUR_WAY_RESTRICTOR:
        set_four_way_restrictor();
        break;

      case OPTION_DPAD_ROTATION:
        set_dpad_rotation(opt.argument);
        break;

      case OPTION_SQUARE_AXIS:
        set_square_axis();
        break;

      case OPTION_HELP_LED:
        opts.mode = Options::PRINT_LED_HELP;
        break;

      case OPTION_DAEMON_DETACH:
        opts.detach = true;
        break;

      case OPTION_DAEMON_PID_FILE:
        opts.pid_file = opt.argument;
        break;

      case OPTION_DAEMON_ON_CONNECT:
        opts.on_connect = opt.argument;
        break;

      case OPTION_DAEMON_ON_DISCONNECT:
        opts.on_disconnect = opt.argument;
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

      case OPTION_LIST_ALL:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_ALL;
        break;

      case OPTION_LIST_ABS:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_ABS;
        break;

      case OPTION_LIST_REL:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_REL;
        break;

      case OPTION_LIST_KEY:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_KEY;
        break;

      case OPTION_LIST_X11KEYSYM:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_X11KEYSYM;
        break;

      case OPTION_LIST_AXIS:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_AXIS;
        break;

      case OPTION_LIST_BUTTON:
        opts.mode = Options::PRINT_ENUMS;
        opts.list_enums |= Options::LIST_BUTTON;
        break;

      case ArgParser::REST_ARG:
        //RAISE_EXCEPTION("unknown command line option: " << opt.argument);
        opts.exec.push_back(opt.argument);
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
    << "This is free software, and you are welcome to redistribute it under certain\n"
    << "conditions; see the file COPYING for details.\n";
}

void
CommandLineParser::set_modifier(const std::string& name, const std::string& value)
{
  m_options->controller.back().modifier.push_back(ModifierPtr(Modifier::from_string(name, value)));
}

void
CommandLineParser::set_axismap(const std::string& name, const std::string& value)
{
  m_options->controller.back().axismap->add(AxisMapping::from_string(name, value));
}

void
CommandLineParser::set_buttonmap(const std::string& name, const std::string& value)
{
  m_options->controller.back().buttonmap->add(ButtonMapping::from_string(name, value));
}

void
CommandLineParser::set_evdev_absmap(const std::string& name, const std::string& value)
{
  if (!name.empty())
  {
    XboxAxis axis = string2axis(value);

    switch (*name.rbegin())
    {
      case '-': m_options->evdev_absmap.bind_minus( str2abs(name.substr(0, name.length()-1)), axis ); break;
      case '+': m_options->evdev_absmap.bind_plus ( str2abs(name.substr(0, name.length()-1)), axis ); break;
      default:  m_options->evdev_absmap.bind_both ( str2abs(name), axis ); break;
    }
  }
  else
  {
    throw std::runtime_error("invalid evdev-absmap argument: " + name + "=" + value);
  }
}

void
CommandLineParser::set_evdev_keymap(const std::string& name, const std::string& value)
{
  m_options->evdev_keymap[str2key(name)] = string2btn(value);
}

void
CommandLineParser::set_relative_axis(const std::string& name, const std::string& value)
{
  m_options->controller.back().relative_axis_map[string2axis(name)]
    = AxisFilterPtr(new RelativeAxisFilter(boost::lexical_cast<int>(value)));
}

void
CommandLineParser::set_autofire(const std::string& name, const std::string& value)
{
  m_options->controller.back().autofire_map[string2btn(name)]
    = ButtonFilterPtr(new AutofireButtonFilter(boost::lexical_cast<int>(value), 0));
}

void
CommandLineParser::set_calibration(const std::string& name, const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());
  
  if (args.size() != 3)
  {
    throw std::runtime_error("calibration requires MIN:CENTER:MAX as argument");
  }
  else
  {
    m_options->controller.back().calibration_map[string2axis(name)]
      = AxisFilterPtr(new CalibrationAxisFilter(boost::lexical_cast<int>(args[0]), 
                                                boost::lexical_cast<int>(args[1]), 
                                                boost::lexical_cast<int>(args[2])));
  }
}

void
CommandLineParser::set_axis_sensitivity(const std::string& name, const std::string& value)
{
  m_options->controller.back().sensitivity_map[string2axis(name)]
    = AxisFilterPtr(new SensitivityAxisFilter(boost::lexical_cast<float>(value)));
}

void
CommandLineParser::set_deadzone(const std::string& value)
{
  m_options->controller.back().deadzone = to_number(32767, value);
}

void
CommandLineParser::set_deadzone_trigger(const std::string& value)
{
  m_options->controller.back().deadzone_trigger = to_number(255, value);
}

void
CommandLineParser::set_square_axis()
{
  m_options->controller.back().square_axis = true;
}

void
CommandLineParser::set_four_way_restrictor()
{
  m_options->controller.back().four_way_restrictor = true;
}

void
CommandLineParser::set_dpad_rotation(const std::string& value)
{
  int degree = boost::lexical_cast<int>(value);
  degree /= 45;
  degree %= 8;
  if (degree < 0) degree += 8;

  m_options->controller.back().dpad_rotation = degree;
}

void
CommandLineParser::read_config_file(Options* opts, const std::string& filename)
{
  std::cout << "CommandLineParser::read_config_file: " << filename << std::endl;
  std::ifstream in(filename.c_str());
  if (!in)
  {
    std::ostringstream str;
    str << "Couldn't open " << filename;
    throw std::runtime_error(str.str());
  }
  else
  {
    INISchemaBuilder builder(m_ini);
    INIParser parser(in, builder, filename);
    parser.run();
  }
}

void
CommandLineParser::read_alt_config_file(Options* opts, const std::string& filename)
{
  opts->controller.push_back(ControllerOptions());
  read_config_file(opts, filename);
}

/* EOF */
