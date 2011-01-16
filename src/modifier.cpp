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

#include "modifier/autofire_modifier.hpp"
#include "modifier/deadzone_modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/axis_sensitivty_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/relativeaxis_modifier.hpp"
#include "modifier/calibration_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

ModifierPtr
Modifier::create(const std::string& str)
{
  return ModifierPtr();
}

/* EOF */
