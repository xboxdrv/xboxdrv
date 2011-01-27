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

#include "modifier.hpp"

#include <boost/tokenizer.hpp>

#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"
#include "modifier/rotate_axis_modifier.hpp"

Modifier*
Modifier::from_string(const std::string& name, const std::string& value)
{
  if (name == "axismap")
  {
    //return AxismapModifier::from_string(value);
    throw std::runtime_error("unknown modifier: " + name);
  }
  else if (name == "buttonmap")
  {
    //return ButtonmapModifier::from_string(value);
    throw std::runtime_error("unknown modifier: " + name);
  }
  else
  {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

    std::vector<std::string> args(tokens.begin(), tokens.end());

    if (name == "dpad-rotation" || name == "dpad-rotate")
    {
      return DpadRotationModifier::from_string(args);
    }
    else if (name == "4wayrest" || name == "four-way-restrictor")
    {
      return FourWayRestrictorModifier::from_string(args);
    }
    else if (name == "square" || name == "square-axis")
    {
      return SquareAxisModifier::from_string(args);
    }
    else if (name == "rotate")
    {
      return RotateAxisModifier::from_string(args);
    }
    else
    {
      throw std::runtime_error("unknown modifier: " + name);
    }
  }
}

  /* EOF */
