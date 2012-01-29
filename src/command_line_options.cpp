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

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
#include <iterator>

#include "evdev_helper.hpp"
#include "helper.hpp"
#include "ini_parser.hpp"
#include "ini_schema_builder.hpp"
#include "options.hpp"
#include "path.hpp"
#include "raise_exception.hpp"
#include "ui_event.hpp"

#include "axis_map_option.hpp"
#include "button_event_factory.hpp"
#include "button_map_option.hpp"

#include "axisfilter/relative_axis_filter.hpp"
#include "axisfilter/calibration_axis_filter.hpp"
#include "axisfilter/sensitivity_axis_filter.hpp"
#include "buttonfilter/autofire_button_filter.hpp"

#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"

#include "xboxdrv_vfs.hpp"

enum {
  OPTION_HELP,
  OPTION_VERBOSE,
  OPTION_VERSION,
  OPTION_DEBUG,
  OPTION_QUIET,
  OPTION_SILENT,
  OPTION_USB_DEBUG,
  OPTION_DAEMON,
  OPTION_CONFIG_OPTION,
  OPTION_CONFIG,
  OPTION_ALT_CONFIG,
  OPTION_WRITE_CONFIG,
  OPTION_TEST_RUMBLE,
  OPTION_RUMBLE,
  OPTION_FF_DEVICE,
  OPTION_PRIORITY,
  OPTION_QUIT,
  OPTION_NO_UINPUT,
  OPTION_MIMIC_XPAD,
  OPTION_MIMIC_XPAD_WIRELESS,
  OPTION_NO_EXTRA_DEVICES,
  OPTION_NO_EXTRA_EVENTS,
  OPTION_TYPE,
  OPTION_FORCE_FEEDBACK,
  OPTION_RUMBLE_GAIN,
  OPTION_MODIFIER,
  OPTION_BUTTONMAP,
  OPTION_AXISMAP,
  OPTION_DEVICE_NAME,
  OPTION_DEVICE_NAMES,
  OPTION_DEVICE_USBID,
  OPTION_DEVICE_USBIDS,
  OPTION_NEXT_CONFIG,
  OPTION_NEXT_CONTROLLER,
  OPTION_CONFIG_SLOT,
  OPTION_CONTROLLER_SLOT,
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
  OPTION_GENERIC_USB_SPEC,
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
  OPTION_EVDEV_RELMAP,
  OPTION_WIIMOTE,
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
  OPTION_DAEMON_MATCH,
  OPTION_DAEMON_MATCH_GROUP,
  OPTION_DAEMON_NO_DBUS,
  OPTION_DAEMON_DBUS,
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
  m_options(),
  m_directory_context()
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
    .add_option(OPTION_DEBUG,         0,  "debug",   "",  "be even more verbose then --verbose")
    .add_option(OPTION_SILENT,       's', "silent",  "",  "do not display events on console")
    .add_option(OPTION_QUIET,         0,  "quiet",   "",  "do not display startup text")
    .add_option(OPTION_USB_DEBUG,     0,  "usb-debug", "",  "enable log messages from libusb")
    .add_option(OPTION_PRIORITY,      0,  "priority", "PRI", "increases process priority (default: normal)")
    .add_newline()

    .add_text("List Options: ")
    .add_option(OPTION_LIST_SUPPORTED_DEVICES, 0, "list-supported-devices", "", "list supported devices (used by xboxdrv-daemon.py)", false)
    .add_option(OPTION_LIST_SUPPORTED_DEVICES_XPAD, 0, "list-supported-devices-xpad", "", "list supported devices in xpad.c style", false)
    .add_option(OPTION_HELP_LED,      0,  "help-led",     "", "list possible values for the led")
    .add_option(OPTION_HELP_DEVICES,  0,  "help-devices", "", "list supported devices")
    .add_option(OPTION_LIST_ABS,       0, "help-abs",       "", "list all possible EV_ABS names")
    .add_option(OPTION_LIST_REL,       0, "help-rel",       "", "list all possible EV_REL names")
    .add_option(OPTION_LIST_KEY,       0, "help-key",       "", "list all possible EV_KEY names")
    .add_option(OPTION_LIST_X11KEYSYM, 0, "help-x11keysym", "", "list all possible X11KeySym")
    .add_option(OPTION_LIST_AXIS,      0, "help-axis",      "", "list all possible XboxAxis")
    .add_option(OPTION_LIST_BUTTON,    0, "help-button",    "", "list all possible XboxButton")
    .add_option(OPTION_LIST_ALL,       0, "help-all",       "", "list all symbols above")
    .add_newline()

    .add_text("Config File Options: ")
    .add_option(OPTION_CONFIG,       'c', "config",      "FILE", "read configuration from FILE")
    .add_option(OPTION_ALT_CONFIG,    0, "alt-config",   "FILE", "read alternative configuration from FILE ")
    .add_option(OPTION_CONFIG_OPTION,'o', "option",      "NAME=VALUE", "Set the given configuration option") 
    .add_option(OPTION_WRITE_CONFIG,  0, "write-config", "FILE", "write an example configuration to FILE")
    .add_newline()

    .add_text("Daemon Options: ")
    .add_option(OPTION_DAEMON,        'D', "daemon",    "", "Run as daemon")
    .add_option(OPTION_DAEMON_DETACH,   0, "detach",      "", "Detach the daemon from the current shell")
    .add_option(OPTION_DAEMON_PID_FILE, 0, "pid-file",    "FILE", "Write daemon pid to FILE")
    .add_option(OPTION_DAEMON_NO_DBUS,  0, "no-dbus",    "", "Disables D-Bus support in the daemon", false)
    .add_option(OPTION_DAEMON_DBUS,     0, "dbus",    "MODE", "Set D-Bus mode (auto, system, session, disabled)")
    .add_option(OPTION_DAEMON_ON_CONNECT,    0, "on-connect", "FILE", "Launch EXE when a new controller is connected")
    .add_option(OPTION_DAEMON_ON_DISCONNECT, 0, "on-disconnect", "FILE", "Launch EXE when a controller is disconnected")
    .add_newline()

    .add_text("Device Options: ")
    .add_option(OPTION_LIST_CONTROLLER, 'L', "list-controller", "", "list available controllers")
    .add_option(OPTION_ID,           'i', "id",      "N", "use controller with id N (default: 0)")
    .add_option(OPTION_WID,          'w', "wid",     "N", "use wireless controller with wid N (default: 0)")
    .add_option(OPTION_DEVICE_BY_PATH, 0, "device-by-path", "BUS:DEV", "Use device BUS:DEV, do not do any scanning")
    .add_option(OPTION_DEVICE_BY_ID,   0, "device-by-id",   "VENDOR:PRODUCT", "Use device that matches VENDOR:PRODUCT (as returned by lsusb)")
    .add_option(OPTION_TYPE,           0, "type",    "TYPE", "Ignore autodetection and enforce controller type (xbox, xbox-mat, xbox360, xbox360-wireless, xbox360-guitar)")
    .add_option(OPTION_DETACH_KERNEL_DRIVER, 'd', "detach-kernel-driver", "", "Detaches the kernel driver currently associated with the device")
    .add_option(OPTION_GENERIC_USB_SPEC, 0, "generic-usb-spec", "SPEC", "Specification for generic USB device")
    .add_newline()

    .add_text("Evdev Options: ")
    .add_option(OPTION_EVDEV,          0, "evdev",   "DEVICE", "Read events from a evdev device, instead of USB")
    .add_option(OPTION_EVDEV_DEBUG,    0, "evdev-debug", "", "Print out all events received from evdev")
    .add_option(OPTION_EVDEV_NO_GRAB,  0, "evdev-no-grab", "", "Do not grab the event device, allow other apps to receive events")
    .add_option(OPTION_EVDEV_ABSMAP,   0, "evdev-absmap", "MAP", "Set how evdev abs names are mapped to xboxdrv names")
    .add_option(OPTION_EVDEV_KEYMAP,   0, "evdev-keymap", "MAP", "Set how evdev abs names are mapped to xboxdrv names")
    .add_option(OPTION_EVDEV_RELMAP,   0, "evdev-relmap", "MAP", "Set how evdev abs names are mapped to xboxdrv names")
    .add_newline()

    .add_text("Wiimote Options: ")
    .add_option(OPTION_WIIMOTE,   0, "wiimote", "", "Use Wiimote as main controller")

    .add_text("Status Options: ")
    .add_option(OPTION_LED,     'l', "led",    "STATUS", "set LED status, see --help-led for possible values")
    .add_option(OPTION_RUMBLE,  'r', "rumble", "L,R", "set the speed for both rumble motors [0-255] (default: 0,0)")
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
    .add_option(OPTION_FF_DEVICE,          0, "ff-device", "DEV", "select to which evdev the force feedback should be connected (default: joystick)")
    .add_newline()

    .add_text("Controller Slot Options: ")
    .add_option(OPTION_CONTROLLER_SLOT,    0, "controller-slot", "N", "Use controller slot N")
    .add_option(OPTION_NEXT_CONTROLLER,    0, "next-controller", "", "Create a new controller entry")
    .add_option(OPTION_DAEMON_MATCH,       0, "match", "RULES",   "Only allow controllers that match any of RULES")
    .add_option(OPTION_DAEMON_MATCH_GROUP, 0, "match-group", "RULES", "Only allow controllers that match all of RULES")
    .add_newline()

    .add_text("Config Slot Options: ")
    .add_option(OPTION_CONFIG_SLOT,        0, "config-slot",     "N", "Use configuration slot N")
    .add_option(OPTION_NEXT_CONFIG,        0, "ui-new", "", "", false) // backward compatibility
    .add_option(OPTION_NEXT_CONFIG,        0, "next-config", "", "Create a new configuration entry")
    .add_option(OPTION_TOGGLE,             0, "toggle", "BTN", "Set button to use for toggling between configs")
    .add_option(OPTION_TOGGLE,             0, "ui-toggle", "BTN", "", false) // backward compatibility
    .add_newline()

    .add_text("Configuration Options:")
    .add_option(OPTION_MODIFIER,          'm', "modifier",       "MOD=ARG:..", "Add a modifier to the modifier spec")
    .add_option(OPTION_TIMEOUT,            0, "timeout",         "INT",  "Amount of time to wait fo a device event before processing autofire, etc. (default: 25)")
    .add_option(OPTION_BUTTONMAP,         'b', "buttonmap",      "MAP",   "Remap the buttons as specified by MAP (example: B=A,X=A,Y=A)")
    .add_option(OPTION_AXISMAP,           'a', "axismap",        "MAP",   "Remap the axis as specified by MAP (example: -Y1=Y1,X1=X2)")
    .add_newline()

    .add_text("Modifier Preset Options: ")
    .add_option(OPTION_AUTOFIRE,           0, "autofire",         "MAP",  "Cause the given buttons to act as autofire (example: A=250)")
    .add_option(OPTION_AXIS_SENSITIVITY,   0, "axis-sensitivity", "MAP",  "Adjust the axis sensitivity (example: X1=2.0,Y1=1.0)")
    .add_option(OPTION_CALIBRARIOTION,     0, "calibration",      "MAP",  "Changes the calibration for the given axis (example: X2=-32768:0:32767)")
    .add_option(OPTION_DEADZONE,           0, "deadzone",         "INT",  "Threshold under which axis events are ignored (default: 0)")
    .add_option(OPTION_DEADZONE_TRIGGER,   0, "deadzone-trigger", "INT",  "Threshold under which trigger events are ignored (default: 0)")
    .add_option(OPTION_DPAD_ROTATION,      0, "dpad-rotation",    "DEGREE", "Rotate the dpad by the given DEGREE, must be a multiple of 45")
    .add_option(OPTION_FOUR_WAY_RESTRICTOR,0, "four-way-restrictor", "",  "Restrict axis movement to one axis at a time")
    .add_option(OPTION_RELATIVE_AXIS,      0, "relative-axis",    "MAP",  "Make an axis emulate a joystick throttle (example: y2=64000)")
    .add_option(OPTION_SQUARE_AXIS,        0, "square-axis",       "",     "Cause the diagonals to be reported as (1,1) instead of (0.7, 0.7)")
    .add_newline()

    .add_text("Uinput Preset Configuration Options: ")
    .add_option(OPTION_TRIGGER_AS_BUTTON,  0, "trigger-as-button", "",    "LT and RT send button instead of axis events")
    .add_option(OPTION_TRIGGER_AS_ZAXIS,   0, "trigger-as-zaxis", "",     "Combine LT and RT to form a zaxis instead")
    .add_option(OPTION_DPAD_AS_BUTTON,     0, "dpad-as-button",   "",     "DPad sends button instead of axis events")
    .add_option(OPTION_DPAD_ONLY,          0, "dpad-only",        "",     "Both sticks are ignored, only DPad sends out axis events")
    .add_option(OPTION_GUITAR,             0, "guitar",            "",     "Enables guitar button and axis mapping")
    .add_option(OPTION_MOUSE,              0, "mouse",            "",     "Enable mouse emulation")
    .add_option(OPTION_MIMIC_XPAD,         0,  "mimic-xpad",  "", "Causes xboxdrv to use the same axis and button names as the xpad kernel driver for wired gamepads")
    .add_option(OPTION_MIMIC_XPAD_WIRELESS, 0,  "mimic-xpad-wireless",  "", "Causes xboxdrv to use the same axis and button names as the xpad kernel driver for wireless gamepads")
    .add_newline()

    .add_text("Uinput Configuration Options: ")
    .add_option(OPTION_NO_UINPUT,          0, "no-uinput",   "", "do not try to start uinput event dispatching")
    .add_option(OPTION_NO_EXTRA_DEVICES,   0, "no-extra-devices",  "", "Do not create separate virtual keyboard and mouse devices, just use a single virtual device")
    .add_option(OPTION_NO_EXTRA_EVENTS,    0, "no-extra-events",  "", "Do not create dummy events to facilitate device type detection")
    .add_option(OPTION_DEVICE_NAME,        0, "device-name",     "NAME", "Changes the name prefix used for devices in the current slot")
    .add_option(OPTION_DEVICE_NAMES,       0, "device-names",    "DEVID=NAME,...", "Changes the descriptive name the given devices")
    .add_option(OPTION_DEVICE_USBID,       0, "device-usbid",     "VENDOR:PRODUCT:VERSION", "Changes the USB Id used for devices in the current slot")
    .add_option(OPTION_DEVICE_USBIDS,      0, "device-usbids",    "DEVID=VENDOR:PRODUCT:VERSION,...", "Changes the USB Id for the given devices")
    .add_option(OPTION_UI_CLEAR,           0, "ui-clear",         "",     "Removes all existing uinput bindings")
    .add_option(OPTION_UI_BUTTONMAP,       0, "ui-buttonmap",     "MAP",  "Changes the uinput events send when hitting a button (example: X=BTN_Y,A=KEY_A)")
    .add_option(OPTION_UI_AXISMAP,         0, "ui-axismap",       "MAP",  "Changes the uinput events send when moving a axis (example: X1=ABS_X2)")
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

    .add_text("Modifier:")
    .add_pseudo("  btn2axis=BTN:BTN:AXIS", "Turns two buttons into an axis")
    .add_pseudo("  dpad-rotate=DEGREE", "Rotate the dpad by the given number of degree")
    .add_pseudo("  dpad-restrictor=RESTRICTION", "Restrict dpad movment to 'x-axis', 'y-axis' or 'four-way'")
    .add_pseudo("  4wayrest, four-way-restrictor=XAXIS:YAXIS", "Restrict the given stick to four directions")
    .add_pseudo("  square, square-axis=XAXIS:YAXIS", "Convert the circular motion range of the given stick to a square one")
    .add_pseudo("  rotate=XAXIS:YAXIS:DEGREE[:MIRROR]", "Rotate the given stick by DEGREE, optionally also mirror it")
    .add_newline()
  
    .add_text("See README for more documentation and examples.")
    .add_text("Report bugs to Ingo Ruhnke <grumbel@gmail.com>");
}

void
CommandLineParser::init_ini(Options* opts)
{
  m_ini.clear();

  m_ini.section("xboxdrv")
    ("verbose", boost::bind(&Options::set_verbose, opts), boost::function<void ()>())
    ("silent", &opts->silent)
    ("quiet",  &opts->quiet)
    ("usb-debug",  &opts->usb_debug)
    ("rumble", &opts->rumble)
    ("led", boost::bind(&Options::set_led, opts, _1))
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
    ("config", boost::bind(&CommandLineParser::read_config_file, this, _1))
    ("alt-config", boost::bind(&CommandLineParser::read_alt_config_file, this, _1))
    ("timeout", &opts->timeout)
    ("priority", boost::bind(&Options::set_priority, opts, _1))
    ("next", boost::bind(&Options::next_config, opts), boost::function<void ()>())
    ("next-controller", boost::bind(&Options::next_controller, opts), boost::function<void ()>())
    ("extra-devices", &opts->extra_devices)
    ("extra-events", &opts->extra_events)
    ("toggle", boost::bind(&Options::set_toggle_button, opts, _1))
    ("ff-device", boost::bind(&Options::set_ff_device, opts, _1))

    ("deadzone", boost::bind(&CommandLineParser::set_deadzone, this, _1))
    ("deadzone-trigger", boost::bind(&CommandLineParser::set_deadzone_trigger, this, _1))
    ("square-axis", boost::bind(&CommandLineParser::set_square_axis, this), boost::function<void ()>())
    ("four-way-restrictor", boost::bind(&CommandLineParser::set_four_way_restrictor, this), boost::function<void ()>())
    ("dpad-rotation", boost::bind(&CommandLineParser::set_dpad_rotation, this, _1))

    // uinput stuff
    ("device-name",       boost::bind(&Options::set_device_name, opts, _1))
    ("device-usbid",      boost::bind(&Options::set_device_usbid, opts, _1))
    ("mouse",             boost::bind(&CommandLineParser::mouse, this), boost::function<void ()>())
    ("guitar",            boost::bind(&Options::set_guitar, opts),            boost::function<void ()>())
    ("trigger-as-button", boost::bind(&Options::set_trigger_as_button, opts), boost::function<void ()>())
    ("trigger-as-zaxis",  boost::bind(&Options::set_trigger_as_zaxis, opts),  boost::function<void ()>())
    ("dpad-as-button",    boost::bind(&Options::set_dpad_as_button, opts),    boost::function<void ()>())
    ("dpad-only",         boost::bind(&Options::set_dpad_only, opts),         boost::function<void ()>())
    ("force-feedback",    boost::bind(&Options::set_force_feedback, opts, _1))
    ("mimic-xpad",        boost::bind(&Options::set_mimic_xpad, opts),        boost::function<void ()>())
    ("mimic-xpad-wireless", boost::bind(&Options::set_mimic_xpad_wireless, opts), boost::function<void ()>())

    ("chatpad",         &opts->chatpad)
    ("chatpad-no-init", &opts->chatpad_no_init)
    ("chatpad-debug",   &opts->chatpad_debug)

    ("headset",         &opts->headset)
    ("headset-debug",   &opts->headset_debug)
    ("headset-dump",    &opts->headset_dump)
    ("headset-play",    &opts->headset_play)
    ("ui-clear",        boost::bind(&Options::set_ui_clear, opts), boost::function<void ()>())
    ;

  m_ini.section("xboxdrv-daemon")
    ("detach",        
     boost::bind(&Options::set_daemon_detach, opts, true),
     boost::bind(&Options::set_daemon_detach, opts, false))
    ("dbus", boost::bind(&Options::set_dbus_mode, opts, _1))
    ("pid-file",      &opts->pid_file)
    ("on-connect",    &opts->on_connect)
    ("on-disconnect", &opts->on_disconnect)
    ;

  m_ini.section("modifier",     boost::bind(&CommandLineParser::set_modifier,     this, _1, _2));
  m_ini.section("ui-buttonmap", boost::bind(&CommandLineParser::set_ui_buttonmap, this, _1, _2));
  m_ini.section("ui-axismap",   boost::bind(&CommandLineParser::set_ui_axismap,   this, _1, _2));

  m_ini.section("buttonmap", boost::bind(&CommandLineParser::set_buttonmap, this, _1, _2));
  m_ini.section("axismap",   boost::bind(&CommandLineParser::set_axismap,   this, _1, _2));

  m_ini.section("autofire",   boost::bind(&CommandLineParser::set_autofire, this, _1, _2));
  m_ini.section("relative-axis",   boost::bind(&CommandLineParser::set_relative_axis, this, _1, _2));
  m_ini.section("calibration",   boost::bind(&CommandLineParser::set_calibration, this, _1, _2));
  m_ini.section("axis-sensitivity",   boost::bind(&CommandLineParser::set_axis_sensitivity, this, _1, _2));
  m_ini.section("device-name", boost::bind(&CommandLineParser::set_device_name, this, _1, _2));
  m_ini.section("device-usbid", boost::bind(&CommandLineParser::set_device_usbid, this, _1, _2));

  for(int controller = 0; controller <= 9; ++controller)
  {
    for(int config = 0; config <= 9; ++config)
    {
      m_ini.section((boost::format("controller%d/config%d/modifier") % controller % config).str(),
                    boost::bind(&CommandLineParser::set_modifier_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/ui-buttonmap") % controller % config).str(),
                    boost::bind(&CommandLineParser::set_ui_buttonmap_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/ui-axismap") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_ui_axismap_n, this, controller, config, _1, _2));

      m_ini.section((boost::format("controller%d/config%d/buttonmap") % controller % config).str(),
                    boost::bind(&CommandLineParser::set_buttonmap_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/axismap") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_axismap_n,   this, controller, config, _1, _2));

      m_ini.section((boost::format("controller%d/config%d/autofire") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_autofire_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/relative-axis") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_relative_axis_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/calibration") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_calibration_n, this, controller, config, _1, _2));
      m_ini.section((boost::format("controller%d/config%d/axis-sensitivity") % controller % config).str(), 
                    boost::bind(&CommandLineParser::set_axis_sensitivity_n, this, controller, config, _1, _2));
    }
  }

  m_ini.section("evdev-absmap", boost::bind(&CommandLineParser::set_evdev_absmap, this, _1, _2));
  m_ini.section("evdev-keymap", boost::bind(&CommandLineParser::set_evdev_keymap, this, _1, _2));
  m_ini.section("evdev-relmap", boost::bind(&CommandLineParser::set_evdev_keymap, this, _1, _2));
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
        opts.set_verbose();
        break;

      case OPTION_QUIET:
        opts.quiet = true;
        break;

      case OPTION_SILENT:
        opts.silent = true;
        break;

      case OPTION_DEBUG:
        opts.set_debug();
        break;

      case OPTION_USB_DEBUG:
        opts.set_usb_debug();
        break;

      case OPTION_PRIORITY:
        opts.set_priority(opt.argument);
        break;

      case OPTION_DAEMON:
        opts.set_daemon();
        break;

      case OPTION_DAEMON_MATCH:
        opts.set_match(opt.argument);
        break;

      case OPTION_DAEMON_MATCH_GROUP:
        opts.set_match_group(opt.argument);
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
        read_config_file(opt.argument);
        break;

      case OPTION_ALT_CONFIG:
        read_alt_config_file(opt.argument);
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
          raise_exception(std::runtime_error, opt.option << " expected an argument in form INT,INT");
        }
        break;

      case OPTION_FF_DEVICE:
        opts.set_ff_device(opt.argument);
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
        opts.set_mimic_xpad();
        break;

      case OPTION_MIMIC_XPAD_WIRELESS:
        opts.set_mimic_xpad_wireless();
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
        else if (opt.argument == "logitech-f310")
        {
          opts.gamepad_type = GAMEPAD_LOGITECH_F310;
        }
        else if (opt.argument == "playstation3-usb")
        {
          opts.gamepad_type = GAMEPAD_PLAYSTATION3_USB;
        }
        else if (opt.argument == "generic-usb")
        {
          opts.gamepad_type = GAMEPAD_GENERIC_USB;
        }
        else
        {
          raise_exception(std::runtime_error, "unknown type: " << opt.argument << '\n'
                          << "Possible types are:\n"
                          << " * xbox\n"
                          << " * xbox-mat\n"
                          << " * xbox360\n"
                          << " * xbox360-guitar\n"
                          << " * xbox360-wireless\n"
                          << " * firestorm\n"
                          << " * firestorm-vsb\n"
                          << " * saitek-p2500\n"
                          << " * logitech-f310\n"
                          << " * generic-usb\n");
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
        opts.get_controller_slot().set_force_feedback(true);
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

      case OPTION_DEVICE_USBID:
        opts.set_device_usbid(opt.argument);
        break;

      case OPTION_DEVICE_USBIDS:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_device_usbid, this, _1, _2));
        break;
                    
      case OPTION_DEVICE_NAME:
        opts.set_device_name(opt.argument);
        break;

      case OPTION_DEVICE_NAMES:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_device_name, this, _1, _2));
        break;

      case OPTION_NEXT_CONFIG:
        opts.next_config();
        break;

      case OPTION_NEXT_CONTROLLER:
        opts.next_controller();
        break;

      case OPTION_CONTROLLER_SLOT:
        opts.controller_slot = boost::lexical_cast<int>(opt.argument);
        break;

      case OPTION_CONFIG_SLOT:
        opts.config_slot = boost::lexical_cast<int>(opt.argument);
        break;

      case OPTION_TOGGLE:
        opts.set_toggle_button(opt.argument);
        break;

      case OPTION_UI_CLEAR:
        opts.set_ui_clear();
        break;

      case OPTION_UI_AXISMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_ui_axismap, this, _1, _2));
        break;

      case OPTION_UI_BUTTONMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_ui_buttonmap, this, _1, _2));
        break;

      case OPTION_MOUSE:
        mouse();
        break;

      case OPTION_GUITAR:
        opts.get_controller_options().uinput.guitar();
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

      case OPTION_EVDEV_RELMAP:
        process_name_value_string(opt.argument, boost::bind(&CommandLineParser::set_evdev_relmap, this, _1, _2));
        break;

      case OPTION_WIIMOTE:
        opts.wiimote = true;
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
          opts.set_led(opt.argument);
        }
        break;

      case OPTION_NO_EXTRA_DEVICES:
        opts.extra_devices = false;
        break;

      case OPTION_NO_EXTRA_EVENTS:
        opts.extra_events = false;
        break;
            
      case OPTION_DPAD_ONLY:
        opts.set_dpad_only();
        break;
            
      case OPTION_DPAD_AS_BUTTON:
        opts.set_dpad_as_button();
        break;

      case OPTION_DEADZONE:
        set_deadzone(opt.argument);
        break;

      case OPTION_DEADZONE_TRIGGER:
        set_deadzone_trigger(opt.argument);
        break;

      case OPTION_TRIGGER_AS_BUTTON:
        opts.set_trigger_as_button();
        break;
        
      case OPTION_TRIGGER_AS_ZAXIS:
        opts.set_trigger_as_zaxis();
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
        opts.set_daemon_detach(true);
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

      case OPTION_DAEMON_DBUS:
        opts.set_dbus_mode(opt.argument);
        break;

      case OPTION_DAEMON_NO_DBUS:
        opts.dbus = Options::kDBusDisabled;
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
            raise_exception(std::runtime_error, opt.option << " expected an argument in form PRODUCT:VENDOR (i.e. 046d:c626)");
          }
          break;
        }

      case OPTION_DEVICE_BY_PATH:
        {
          char busid[4] = { '\0' };
          char devid[4] = { '\0' };

          if (sscanf(opt.argument.c_str(), "%3s:%3s", busid, devid) != 2)
          {  
            raise_exception(std::runtime_error, opt.option << " expected an argument in form BUS:DEV (i.e. 006:003)");
          }
          else
          {
            opts.busid = busid;
            opts.devid = devid;
          }
        }
        break;

      case OPTION_GENERIC_USB_SPEC:
        set_generic_usb_spec(opt.argument);
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
        opts.exec.push_back(opt.argument);
        break;

      default:
        raise_exception(std::runtime_error, "unknown command line option: " << opt.option);
        break;
    }
  }

  options->finish();
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
    << "Copyright © 2008-2011 Ingo Ruhnke <grumbel@gmx.de>\n"
    << "Licensed under GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
    << "This program comes with ABSOLUTELY NO WARRANTY.\n"
    << "This is free software, and you are welcome to redistribute it under certain\n"
    << "conditions; see the file COPYING for details.\n";
}

void
CommandLineParser::set_modifier(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().modifier.push_back(ModifierOption(name, value));
}

void
CommandLineParser::set_device_usbid(const std::string& name, const std::string& value)
{
  uint32_t devid = UInput::parse_device_id(name);
  m_options->uinput_device_usbids[devid] = UInput::parse_input_id(value);
}

void
CommandLineParser::set_device_name(const std::string& name, const std::string& value)
{
  uint32_t devid = UInput::parse_device_id(name);
  m_options->uinput_device_names[devid] = value;
}

void
CommandLineParser::set_ui_buttonmap(const std::string& name, const std::string& value)
{
  set_ui_buttonmap(m_options->get_controller_options().uinput.get_btn_map(),
                   name, value);
}

void
CommandLineParser::set_ui_buttonmap(ButtonMapOptions& btn_map, const std::string& name, const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(name, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  std::vector<std::string> lst(tokens.begin(), tokens.end());

  // boost::tokenizer doesn't return any tokens on an empty string, so
  // add an empty token to the list to make the interpretation work
  // properly
  if (lst.empty())
  {
    lst.push_back(std::string());
  }

  int idx = 0;
  for(std::vector<std::string>::iterator t = lst.begin(); t != lst.end(); ++t, ++idx)
  {
    switch(idx)
    { 
      case 0: // shift+key portion
        btn_map.push_back(ButtonMapOption(*t, value));
        break;

      default:
        btn_map.back().add_filter(*t);
        break;
    }
  }
}

void
CommandLineParser::set_ui_axismap(const std::string& name, const std::string& value)
{
  set_ui_axismap(m_options->get_controller_options().uinput.get_axis_map(),
                 name, value);
}

void
CommandLineParser::set_ui_axismap(AxisMapOptions& axis_map, const std::string& name, const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(name, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  std::vector<std::string> lst(tokens.begin(), tokens.end());

  // boost::tokenizer doesn't return any tokens on an empty string, so
  // add an empty token to the list to make the interpretation work
  // properly
  if (lst.empty())
  {
    lst.push_back(std::string());
  }

  int idx = 0;
  for(std::vector<std::string>::iterator t = lst.begin(); t != lst.end(); ++t, ++idx)
  {
    switch(idx)
    { 
      case 0: // shift+key portion
        axis_map.push_back(AxisMapOption(*t, value));
        break;

      default:
        axis_map.back().add_filter(*t);
        break;
    }
  }
}

void
CommandLineParser::set_axismap(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().axismap.push_back(AxisMappingOption(name, value));
}

void
CommandLineParser::set_buttonmap(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().buttonmap.push_back(ButtonMappingOption(name, value));
}

void
CommandLineParser::set_evdev_absmap(const std::string& name, const std::string& value)
{
  m_options->evdev_absmap[str2abs(name)] = value;
}

void
CommandLineParser::set_evdev_keymap(const std::string& name, const std::string& value)
{
  m_options->evdev_keymap[str2key(name)] = value;
}

void
CommandLineParser::set_evdev_relmap(const std::string& name, const std::string& value)
{
  m_options->evdev_relmap[str2rel(name)] = value;
}

void
CommandLineParser::set_relative_axis(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().relative_axis_map[name]
    = AxisFilterPtr(new RelativeAxisFilter(boost::lexical_cast<int>(value)));
}

void
CommandLineParser::set_autofire(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().autofire_map[name]
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
    m_options->get_controller_options().calibration_map[name]
      = AxisFilterPtr(new CalibrationAxisFilter(boost::lexical_cast<int>(args[0]), 
                                                boost::lexical_cast<int>(args[1]), 
                                                boost::lexical_cast<int>(args[2])));
  }
}

void
CommandLineParser::set_axis_sensitivity(const std::string& name, const std::string& value)
{
  m_options->get_controller_options().sensitivity_map[name]
    = AxisFilterPtr(new SensitivityAxisFilter(boost::lexical_cast<float>(value)));
}

void
CommandLineParser::set_deadzone(const std::string& value)
{
  m_options->get_controller_options().deadzone = to_number(32767, value);
}

void
CommandLineParser::set_deadzone_trigger(const std::string& value)
{
  m_options->get_controller_options().deadzone_trigger = to_number(255, value);
}

void
CommandLineParser::set_square_axis()
{
  m_options->get_controller_options().square_axis = true;
}

void
CommandLineParser::set_four_way_restrictor()
{
  m_options->get_controller_options().four_way_restrictor = true;
}

void
CommandLineParser::set_dpad_rotation(const std::string& value)
{
  int degree = boost::lexical_cast<int>(value);
  degree /= 45;
  degree %= 8;
  if (degree < 0) degree += 8;

  m_options->get_controller_options().dpad_rotation = degree;
}

void
CommandLineParser::read_buildin_config_file(const std::string& filename, 
                                            const char* data, unsigned int data_len)
{
  log_info("reading 'buildin://" << filename << "'");

  std::string str(data, data_len);
  std::istringstream in(str);
  if (!in)
  {
    raise_exception(std::runtime_error, "couldn't open: buildin://" << filename);
  }
  else
  {
    INISchemaBuilder builder(m_ini);
    INIParser parser(in, builder, filename);
    parser.run();
  }
}

void
CommandLineParser::read_config_file(const std::string& filename)
{
  log_info("reading '" << filename << "'");

  std::ifstream in(filename.c_str());
  if (!in)
  {
    raise_exception(std::runtime_error, "couldn't open: " << filename);
  }
  else
  {
    m_directory_context.push_back(path::dirname(filename));

    INISchemaBuilder builder(m_ini);
    INIParser parser(in, builder, filename);
    parser.run();

    m_directory_context.pop_back();
  }
}

void
CommandLineParser::read_alt_config_file(const std::string& filename)
{
  m_options->next_config();
  read_config_file(filename);
}

void
CommandLineParser::set_ui_buttonmap_n(int controller, int config, const std::string& name, const std::string& value)
{
  set_ui_buttonmap(m_options->controller_slots[controller].get_options(config).uinput.get_btn_map(),
                   name, value);
}

void
CommandLineParser::set_ui_axismap_n(int controller, int config, const std::string& name, const std::string& value)
{
  set_ui_axismap(m_options->controller_slots[controller].get_options(config).uinput.get_axis_map(),
                 name, value);
}

void
CommandLineParser::set_modifier_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config).modifier.push_back(ModifierOption(name, value));
}

void
CommandLineParser::set_axismap_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config).axismap.push_back(AxisMappingOption(name, value));
}

void
CommandLineParser::set_buttonmap_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config).buttonmap.push_back(ButtonMappingOption(name, value));
}

void
CommandLineParser::set_relative_axis_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config)
    .relative_axis_map[name] = AxisFilterPtr(new RelativeAxisFilter(boost::lexical_cast<int>(value)));
}

void
CommandLineParser::set_autofire_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config)
    .autofire_map[name] = ButtonFilterPtr(new AutofireButtonFilter(boost::lexical_cast<int>(value), 0));
}

void
CommandLineParser::set_calibration_n(int controller, int config, const std::string& name, const std::string& value)
{
  // FIXME: not implemented
  assert(!"implement me");
}

void
CommandLineParser::set_axis_sensitivity_n(int controller, int config, const std::string& name, const std::string& value)
{
  m_options->controller_slots[controller].get_options(config)
    .sensitivity_map[name] = AxisFilterPtr(new SensitivityAxisFilter(boost::lexical_cast<float>(value)));
}

void
CommandLineParser::mouse()
{
  read_buildin_config_file("examples/mouse.xboxdrv",
                           xboxdrv_vfs::examples_mouse_xboxdrv,
                           sizeof(xboxdrv_vfs::examples_mouse_xboxdrv));
}

void
CommandLineParser::set_generic_usb_spec(const std::string& spec)
{
  m_options->m_generic_usb_specs.push_back(Options::GenericUSBSpec::from_string(spec));
}

std::string
CommandLineParser::get_directory_context() const
{
  if (m_directory_context.empty())
  {
    return std::string();
  }
  else
  {
    return m_directory_context.back();
  }
}

/* EOF */
