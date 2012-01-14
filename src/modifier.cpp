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
#include <linux/input.h>

#include "modifier/acc2axis_modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/button2axis_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/dpad_restrictor_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/ir2axis_modifier.hpp"
#include "modifier/join_axis_modifier.hpp"
#include "modifier/key_copy_modifier.hpp"
#include "modifier/log_modifier.hpp"
#include "modifier/rotate_axis_modifier.hpp"
#include "modifier/split_axis_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"
#include "modifier/statistic_modifier.hpp"
#include "modifier/stick_zone_modifier.hpp"

#include "evdev_helper.hpp"
#include "raise_exception.hpp"

Modifier*
Modifier::from_string(const std::string& name, const std::string& value, 
                      const ControllerMessageDescriptor& msg_desc)
{
  if (name == "axismap")
  {
    return AxismapModifier::from_string(value, msg_desc);
  }
  else if (name == "buttonmap" || name == "btnmap")
  {
    return ButtonmapModifier::from_string(value, msg_desc);
  }
  else
  {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

    std::vector<std::string> args(tokens.begin(), tokens.end());

    if (name == "dpad-rotation" || name == "dpad-rotate")
    {
      return DpadRotationModifier::from_string(args, msg_desc);
    }
    else if (name == "4wayrest" || name == "four-way-restrictor")
    {
      return FourWayRestrictorModifier::from_string(args, msg_desc);
    }
    else if (name == "square" || name == "square-axis")
    {
      return SquareAxisModifier::from_string(args, msg_desc);
    }
    else if (name == "rotate")
    {
      return RotateAxisModifier::from_string(args, msg_desc);
    }
    else if (name == "stat" || name == "statistic")
    {
      return StatisticModifier::from_string(args, msg_desc);
    }
    else if (name == "dpad-restrictor")
    {
      return DpadRestrictorModifier::from_string(args, msg_desc);
    }
    else if (name == "btn2axis" || name == "button2axis")
    {
      return Button2AxisModifier::from_string(args, msg_desc);
    }
    else if (name == "acc2axis")
    {
      return Acc2AxisModifier::from_string(args, msg_desc);
    }
    else if (name == "ir2axis")
    {
      return IR2AxisModifier::from_string(args, msg_desc);
    }
    else if (name == "stickzone")
    {
      return StickZoneModifier::from_string(args, msg_desc);
    }
    else if (name == "split-axis" || name == "splitaxis")
    {
      return SplitAxisModifier::from_string(args, msg_desc);
    }
    else if (name == "join-axis" || name == "joinaxis")
    {
      return JoinAxisModifier::from_string(args, msg_desc);
    }
    else if (name == "copy")
    {
      if (args.size() < 2)
      {
        raise_exception(std::runtime_error, "'copy' modifier requires at least two arguments");
      }
      else
      {
        switch(get_event_type(args[0]))
        {
          case EV_REL:
          case EV_ABS:
            raise_exception(std::runtime_error, "not implemented");
            break;

          case EV_KEY:
            return KeyCopyModifier::from_string(args, msg_desc);

          default:
            raise_exception(std::runtime_error, "'copy' modifier couldn't guess");
        }
      }
    }
    else if (name == "key-copy")
    {
      return KeyCopyModifier::from_string(args, msg_desc);
    }
    else if (name == "log")
    {
      return LogModifier::from_string(value, msg_desc);
    }
    else
    {
      throw std::runtime_error("unknown modifier: " + name);
    }
  }
}

  /* EOF */
