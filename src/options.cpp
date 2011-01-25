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

#include "options.hpp"

#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"

Options* g_options;

Options::Options() :
  mode(RUN_DEFAULT),
  verbose(false),
  silent (false),
  quiet  (false),
  rumble (false),
  led    (-1),
  rumble_l(-1),
  rumble_r(-1),
  rumble_gain(255),
  controller_id(0),
  wireless_id(0),
  instant_exit(false),
  no_uinput(false),
  detach_kernel_driver(),
  timeout(25),
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
  pid_file(),
  on_connect(),
  on_disconnect(),
  exec(),
  list_enums(0),
  config_toggle_button(XBOX_BTN_UNKNOWN),
  controller_slot(0),
  config_slot(0),
  extra_devices(true)
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

  // FIXME: move this somewhere else
  if (config_toggle_button == XBOX_BTN_UNKNOWN)
  {
    config_toggle_button = XBOX_BTN_GUIDE;
  }

  // create the entry if not already available
  controller_slots[controller_slot].get_options(config_slot);
}

void
Options::set_device_name(const std::string& name)
{
  //get_controller().uinput.mouse();
}

void
Options::set_mouse()
{
  get_controller_options().uinput.mouse();
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
}

void
Options::set_dpad_only()
{
}

void
Options::set_force_feedback()
{
}

void
Options::set_mimic_xpad()
{
  get_controller_options().uinput.mimic_xpad();
}

void
Options::add_match(const std::string& lhs, const std::string& rhs)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(rhs, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (lhs == "usbid")
  {
    if (args.size() != 2)
    {
      raise_exception(std::runtime_error, "usbid requires VENDOR:PRODUCT argument");
    }
    else
    {
      int vendor  = hexstr2int(args[0]);
      int product = hexstr2int(args[1]);
      get_controller_slot().add_match_rule(ControllerMatchRule::match_usb_id(vendor, product));
    }
  }
  else if (lhs == "usbpath")
  {
    if (args.size() != 2)
    {
      raise_exception(std::runtime_error, "usbpath requires BUS:DEV argument");
    }
    else
    {
      int bus = boost::lexical_cast<int>(args[0]);
      int dev = boost::lexical_cast<int>(args[1]);
      get_controller_slot().add_match_rule(ControllerMatchRule::match_usb_path(bus, dev));
    }
  }
  else if (lhs == "evdev")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "evdev rule requires PATH argument");
    }
    else
    {
      get_controller_slot().add_match_rule(ControllerMatchRule::match_evdev_path(args[0]));
    }
  }
  else
  {
    raise_exception(std::runtime_error, "'" << lhs << "' not a valid match rule name");
  }
}

void
Options::set_match(const std::string& str)
{
  process_name_value_string(str, boost::bind(&Options::add_match, this, _1, _2));
}

void
Options::set_match_group(const std::string& str)
{
  // FIXME: not implied
  assert(!"not implemented");
}

/* EOF */
