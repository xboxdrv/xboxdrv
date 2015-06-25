/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller_options.hpp"

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

/* EOF */
