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
#include <linux/input.h>

#include "controller_options.hpp"
#include "controller_slot_options.hpp"
#include "evdev_absmap.hpp"
#include "uinput_options.hpp"
#include "xpad_device.hpp"

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
         PRINT_ENUMS,
         PRINT_LED_HELP
  } mode;

  enum {
    LIST_ALL       = ~0,
    LIST_ABS       = (1<<0),
    LIST_REL       = (1<<1),
    LIST_KEY       = (1<<2),
    LIST_X11KEYSYM = (1<<3),
    LIST_AXIS      = (1<<4),
    LIST_BUTTON    = (1<<5)
  };

  enum Priority {
    kPriorityNormal,
    kPriorityRealtime
  };

  // General program options
  bool silent;
  bool quiet;
  bool rumble;
  int  rumble_l;
  int  rumble_r;
  int  rumble_gain;
  int  controller_id;
  int  wireless_id;
  bool instant_exit;
  bool no_uinput;
  bool detach_kernel_driver;
  int  timeout;
  Priority priority;

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
  typedef std::map<int, ControllerSlotOptions> ControllerSlots;
  ControllerSlots controller_slots;

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
  enum DBusSubsystemMode {
    kDBusDisabled, /// disable dbus
    kDBusAuto,     /// choice system or session bus on uid
    kDBusSystem,   /// chose system bus
    kDBusSession   /// chose session bus
  };
  DBusSubsystemMode dbus;
  std::string pid_file;
  std::string on_connect;
  std::string on_disconnect;

  std::vector<std::string> exec;

  uint32_t list_enums;

  XboxButton config_toggle_button;
  bool config_toggle_button_is_set;

  int controller_slot;
  int config_slot;

  bool extra_devices;
  bool extra_events;

  std::map<uint32_t, std::string> uinput_device_names;
  std::map<uint32_t, struct input_id> uinput_device_usbids;

  bool usb_debug;

  struct GenericUSBSpec
  {
  private:
    void apply_pair(const std::string& name,
                    const std::string& value);

    GenericUSBSpec() :
      m_vendor_id(-1),
      m_product_id(-1),
      m_interface(-1),
      m_endpoint(-1),
      m_packetsize(-1)
    {}

  public:
    static GenericUSBSpec from_string(const std::string& str);

    GenericUSBSpec(int vendor_id_,
                   int product_id_,
                   int interface_,
                   int endpoint_,
                   int packetsize_) :
      m_vendor_id(vendor_id_),
      m_product_id(product_id_),
      m_interface(interface_),
      m_endpoint(endpoint_),
      m_packetsize(packetsize_)
    {}

    int m_vendor_id;
    int m_product_id;
    int m_interface;
    int m_endpoint;
    int m_packetsize;
  };

  std::vector<GenericUSBSpec> m_generic_usb_specs;

public:
  Options();

  ControllerSlotOptions& get_controller_slot();
  const ControllerSlotOptions& get_controller_slot() const;

  /** Returns the currently active configuration */
  ControllerOptions& get_controller_options();
  const ControllerOptions& get_controller_options() const;

  void set_priority(const std::string& value);

  void set_ui_clear();

  void next_controller();
  void next_config();

  void set_verbose();
  void set_debug();
  void set_usb_debug();
  void set_quiet();

  void set_dbus_mode(const std::string& value);
  void set_led(const std::string& value);
  void set_device_name(const std::string& name);
  void set_device_usbid(const std::string& name);
  void set_toggle_button(const std::string& toggle);
  void set_guitar();
  void set_trigger_as_button();
  void set_trigger_as_zaxis();
  void set_dpad_as_button();
  void set_dpad_only();
  void set_force_feedback(const std::string& value);
  void set_ff_device(const std::string& value);
  void set_mimic_xpad();
  void set_mimic_xpad_wireless();

  void set_daemon();
  void set_daemon_detach(bool value);

  void add_match(const std::string& lhs, const std::string& rhs);
  void set_match(const std::string& str);
  void set_match_group(const std::string& str);

  GenericUSBSpec find_generic_usb_spec(int vendor_id, int product_id) const;

  void finish();
};

extern Options* g_options;

#endif

/* EOF */
