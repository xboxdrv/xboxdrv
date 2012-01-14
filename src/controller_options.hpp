/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_CONTROLLER_OPTIONS_HPP
#define HEADER_XBOXDRV_CONTROLLER_OPTIONS_HPP

#include <vector>
#include <map>

#include "button_filter.hpp"
#include "controller_message.hpp"
#include "modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "uinput_options.hpp"

class AxismapModifier;
class ButtonmapModifier;

class ControllerOptions
{
public:
  ControllerOptions();

  UInputOptions uinput;
  std::vector<ModifierOption> modifier;

  // everything below gets later converted into modifier
  std::vector<ButtonMappingOption> buttonmap;
  std::vector<AxisMappingOption> axismap;

  int  deadzone;
  int  deadzone_trigger;
  bool square_axis;
  bool four_way_restrictor;
  int  dpad_rotation;

  std::map<XboxAxis, AxisFilterPtr> calibration_map;
  std::map<XboxAxis, AxisFilterPtr> sensitivity_map;
  std::map<XboxAxis, AxisFilterPtr> relative_axis_map;
  std::map<XboxButton, ButtonFilterPtr> autofire_map;
};

#endif

/* EOF */
