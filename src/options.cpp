/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "options.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

Options* g_options;

Options::GenericUSBSpec
Options::GenericUSBSpec::from_string(const std::string& str)
{
  GenericUSBSpec spec;
  process_name_value_string(str, boost::bind(&GenericUSBSpec::apply_pair, boost::ref(spec), _1, _2));
  return spec;
}

void
Options::GenericUSBSpec::apply_pair(const std::string& name,
                                    const std::string& value)
{
  if (name == "if" || name == "interface")
  {
    m_interface = str2int(value);
  }
  else if (name == "ep" || name == "endpoint")
  {
    m_endpoint = str2int(value);
  }
  else if (name == "vid" || name == "vendor_id" || name == "vendorid" || name == "vendor")
  {
    m_vendor_id = hexstr2int(value);
  }
  else if (name == "pid" || name == "product_id" || name == "productid" || name == "product")
  {
    m_product_id = hexstr2int(value);
  }
  else
  {
    raise_exception(std::runtime_error, "unknown name " << name);
  }
}

Options::Options() :
  mode(RUN_DEFAULT),
  silent (false),
  quiet  (false),
  rumble (false),
  rumble_l(-1),
  rumble_r(-1),
  rumble_gain(255),
  controller_id(0),
  wireless_id(0),
  instant_exit(false),
  no_uinput(false),
  detach_kernel_driver(),
  timeout(10),
  priority(kPriorityNormal),
  gamepad_type(GAMEPAD_UNKNOWN),
  busid(),
  devid(),
  vendor_id(-1),
  product_id(-1),
  evdev_device(),
  evdev_absmap(),
  evdev_grab(true),
  evdev_debug(false),
  evdev_keymap(),
  controller_slots(),
  chatpad(false),
  chatpad_no_init(false),
  chatpad_debug(false),
  headset(false),
  headset_debug(false),
  headset_dump(),
  headset_play(),
  detach(false),
  dbus(kDBusAuto),
  pid_file(),
  on_connect(),
  on_disconnect(),
  exec(),
  list_enums(0),
  config_toggle_button(XBOX_BTN_UNKNOWN),
  config_toggle_button_is_set(false),
  controller_slot(0),
  config_slot(0),
  extra_devices(true),
  extra_events(true),
  uinput_device_names(),
  uinput_device_usbids(),
  usb_debug(false),
  m_generic_usb_specs()
{
  // create the entry if not already available
  controller_slots[controller_slot].get_options(config_slot);
}

ControllerSlotOptions&
Options::get_controller_slot()
{
  return controller_slots[controller_slot];
}

const ControllerSlotOptions&
Options::get_controller_slot() const
{
  ControllerSlots::const_iterator it = controller_slots.find(controller_slot);
  if (it == controller_slots.end())
  {
    assert(!"shouldn't happen");
  }
  else
  {
    return it->second;
  }
}

ControllerOptions&
Options::get_controller_options()
{
  return controller_slots[controller_slot].get_options(config_slot);
}

const ControllerOptions&
Options::get_controller_options() const
{
  ControllerSlots::const_iterator it = controller_slots.find(controller_slot);
  if (it == controller_slots.end())
  {
    assert(!"shouldn't happen");
  }
  else
  {
    ControllerSlotOptions::Options::const_iterator cfg = it->second.get_options().find(config_slot);
    if (cfg == it->second.get_options().end())
    {
      assert(!"shouldn't happen either");
    }
    else
    {
      return cfg->second;
    }
  }
}

void
Options::set_priority(const std::string& value)
{
  if (value == "realtime")
  {
    priority = kPriorityRealtime;
  }
  else if (value == "normal")
  {
    priority = kPriorityNormal;
  }
  else
  {
    raise_exception(std::runtime_error, "unknown priority value: '" << value << "'");
  }
}

void
Options::set_ui_clear()
{
  get_controller_options().uinput.get_axis_map().clear();
  get_controller_options().uinput.get_btn_map().clear();
}

void
Options::next_controller()
{
  controller_slot += 1;
  config_slot = 0;

  // create the entry if not already available
  controller_slots[controller_slot].get_options(config_slot);
}

void
Options::next_config()
{
  config_slot += 1;

  // create the entry if not already available
  controller_slots[controller_slot].get_options(config_slot);
}

void
Options::set_verbose()
{
  g_logger.incr_log_level(Logger::kInfo);
}

void
Options::set_debug()
{
  g_logger.incr_log_level(Logger::kDebug);
}

void
Options::set_usb_debug()
{
  usb_debug = true;
}

void
Options::set_dbus_mode(const std::string& value)
{
  if (value == "system")
  {
    dbus = kDBusSystem;
  }
  else if (value == "session")
  {
    dbus = kDBusSession;
  }
  else if (value == "auto")
  {
    dbus = kDBusAuto;
  }
  else if (value == "disabled")
  {
    dbus = kDBusDisabled;
  }
  else
  {
    try {
      // Fallback for backward compatibility
      if (str2bool(value))
      {
        dbus = kDBusAuto;
      }
      else
      {
        dbus = kDBusDisabled;
      }
    }
    catch(const std::exception& err)
    {
      // fallback failed, so assume that the user used the new way and
      // did a typing error
      raise_exception(std::runtime_error, "unknown dbus mode: " << value);
    }
  }
}

void
Options::set_led(const std::string& value)
{
  get_controller_slot().set_led_status(str2int(value));
}

void
Options::set_device_name(const std::string& name)
{
  uint32_t device_id = UInput::create_device_id(controller_slot, DEVICEID_AUTO);
  uinput_device_names[device_id] = name;
}

void
Options::set_device_usbid(const std::string& name)
{
  uint32_t device_id = UInput::create_device_id(controller_slot, DEVICEID_AUTO);
  uinput_device_usbids[device_id] = UInput::parse_input_id(name);
}

void
Options::set_toggle_button(const std::string& str)
{
  if (str == "void")
  {
    config_toggle_button = XBOX_BTN_UNKNOWN;
    config_toggle_button_is_set = true;
  }
  else
  {
    config_toggle_button = string2btn(str);
    config_toggle_button_is_set = true;
  }
}

void
Options::set_guitar()
{
  get_controller_options().uinput.guitar();
}

void
Options::set_trigger_as_button()
{
  get_controller_options().uinput.trigger_as_button();
}

void
Options::set_trigger_as_zaxis()
{
  get_controller_options().uinput.trigger_as_zaxis();
}

void
Options::set_dpad_as_button()
{
  get_controller_options().uinput.dpad_as_button();
}

void
Options::set_dpad_only()
{
  get_controller_options().uinput.dpad_only();
}

void
Options::set_force_feedback(const std::string& value)
{
  get_controller_slot().set_force_feedback(str2bool(value));
}

void
Options::set_ff_device(const std::string& value)
{
  get_controller_slot().set_ff_device(value);
}

void
Options::set_mimic_xpad()
{
  // BTN_BACK is recognized as mouse button, so we have to disallow
  // automatic allocation
  extra_devices = false;
  extra_events  = false;

  set_device_name("Microsoft Xbox 360 pad");
  set_device_usbid("045e:028e:110");
  get_controller_options().uinput.mimic_xpad();
}

void
Options::set_mimic_xpad_wireless()
{
  // BTN_BACK is recognized as mouse button, so we have to disallow
  // automatic allocation
  extra_devices = false;
  extra_events  = false;

  set_device_name("Xbox 360 Wireless Receiver");
  set_device_usbid("045e:0719:100");
  get_controller_options().uinput.mimic_xpad_wireless();
}

void
Options::set_daemon()
{
  mode = RUN_DAEMON;
  silent = true;
}

void
Options::set_daemon_detach(bool value)
{
  detach = value;
  if (detach)
  {
    silent = true;
    quiet  = true;
  }
}

void
Options::set_quiet()
{
  quiet  = true;
  silent = true;
}

void
Options::add_match(const std::string& lhs, const std::string& rhs)
{
  get_controller_slot().add_match_rule(ControllerMatchRule::from_string(lhs, rhs));
}

void
Options::set_match(const std::string& str)
{
  process_name_value_string(str, boost::bind(&Options::add_match, this, _1, _2));
}

void
Options::set_match_group(const std::string& str)
{
  boost::shared_ptr<ControllerMatchRuleGroup> group(new ControllerMatchRuleGroup);

  process_name_value_string(str, boost::bind(&ControllerMatchRuleGroup::add_rule_from_string, group, _1, _2));

  get_controller_slot().add_match_rule(group);
}

Options::GenericUSBSpec
Options::find_generic_usb_spec(int vendor_id_, int product_id_) const
{
  for(std::vector<GenericUSBSpec>::const_iterator i = m_generic_usb_specs.begin(); i != m_generic_usb_specs.end(); ++i)
  {
    //log_tmp(i->m_vendor_id  << " - " << vendor_id_ << " "
    //        << i->m_product_id << " - " << product_id_);

    if (i->m_vendor_id  == vendor_id_ &&
        i->m_product_id == product_id_)
    {
      return *i;
    }
  }

  raise_exception(std::runtime_error, "no matching GenericUSBSpec found for "
                  << boost::format("%04x:%04x") % static_cast<int>(vendor_id_) % static_cast<int>(product_id_));
}

void
Options::finish()
{
  // if we have multiple configurations and the toggle button isn't
  // set, set it to the guide button
  if (!config_toggle_button_is_set &&
      controller_slots[controller_slot].get_options().size() > 1)
  {
    config_toggle_button = XBOX_BTN_GUIDE;
  }
}

/* EOF */
