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

Options* g_options;

ControllerOptions::ControllerOptions() :
  uinput(),
  modifier(),
  buttonmap(new ButtonmapModifier),
  axismap(new AxismapModifier),
  deadzone(0),
  deadzone_trigger(0),
  square_axis(false),
  four_way_restrictor(0),
  dpad_rotation(0),

  calibration_map(),
  sensitivity_map(),
  relative_axis_map(),
  autofire_map()
{
}

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
  config_slot(0)
{
  // create the entry if not already available
  controller_slots[controller_slot][config_slot];
}

Options::ControllerConfigs&
Options::get_controller_slot()
{
  return controller_slots[controller_slot];
}

const Options::ControllerConfigs& 
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
  return controller_slots[controller_slot][config_slot];
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
    ControllerConfigs::const_iterator cfg = it->second.find(config_slot);
    if (cfg == it->second.end())
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
  controller_slots[controller_slot][config_slot];
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
  controller_slots[controller_slot][config_slot];
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

/* EOF */
