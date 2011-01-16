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
  deadzone(0),
  deadzone_trigger(0),
  button_map(),
  axis_map(),
  autofire_map(),
  relative_axis_map(),
  calibration_map(),
  axis_sensitivity_map(),
  square_axis(false),
  four_way_restrictor(false),
  dpad_rotation(0)
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
  gamepad_type(GAMEPAD_UNKNOWN),
  vendor_id(-1),
  product_id(-1),
  evdev_device(),
  evdev_absmap(),
  evdev_grab(true),
  evdev_debug(false),
  chatpad(false),
  chatpad_no_init(false),
  chatpad_debug(false),
  headset(false),
  headset_debug(false),
  headset_dump(),
  headset_play(),
  detach(false),
  pid_file()
{
}

/* EOF */
