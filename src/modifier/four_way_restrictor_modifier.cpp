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

#include "four_way_restrictor_modifier.hpp"

FourWayRestrictorModifier*
FourWayRestrictorModifier::from_string(const std::vector<std::string>& args)
{
  return new FourWayRestrictorModifier;
}

FourWayRestrictorModifier::FourWayRestrictorModifier()
{
}

void
FourWayRestrictorModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  // left Stick
  if (abs(get_axis(msg, XBOX_AXIS_X1)) > abs(get_axis(msg, XBOX_AXIS_Y1)))
  {
    set_axis(msg, XBOX_AXIS_Y1, 0);
  }
  else if (abs(get_axis(msg, XBOX_AXIS_Y1)) > abs(get_axis(msg, XBOX_AXIS_X1)))
  {
    set_axis(msg, XBOX_AXIS_X1, 0);
  }
  else
  {
    set_axis(msg, XBOX_AXIS_X1, 0);
  }

  // right stick
  if (abs(get_axis(msg, XBOX_AXIS_X2)) > abs(get_axis(msg, XBOX_AXIS_Y2)))
  {
    set_axis(msg, XBOX_AXIS_Y2, 0);
  }
  else if (abs(get_axis(msg, XBOX_AXIS_Y2)) > abs(get_axis(msg, XBOX_AXIS_X2)))
  {
    set_axis(msg, XBOX_AXIS_X2, 0);
  }
  else
  {
    set_axis(msg, XBOX_AXIS_X2, 0);
  }
}

/* EOF */
