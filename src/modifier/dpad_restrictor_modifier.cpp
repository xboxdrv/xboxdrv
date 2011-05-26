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

#include "dpad_restrictor_modifier.hpp"

#include <boost/tokenizer.hpp>
#include <stdexcept>

#include "raise_exception.hpp"

DpadRestrictorModifier*
DpadRestrictorModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 1)
  {
    raise_exception(std::runtime_error, "one argument required");
  }
  else
  {
    if (args[0] == "xy" || args[0] == "fourway" || args[0] == "four-way")
    {
      return new DpadRestrictorModifier(kRestrictFourWay);
    }
    else if (args[0] == "x" || args[0] == "x-axis" || args[0] == "xaxis" || args[0] == "horz" || args[0] == "horizontal")
    {
      return new DpadRestrictorModifier(kRestrictXAxis);
    }
    else if (args[0] == "y" || args[0] == "y-axis" || args[0] == "yaxis" || args[0] == "vert" || args[0] == "vertical")
    {
      return new DpadRestrictorModifier(kRestrictYAxis);
    }
    else
    {
      raise_exception(std::runtime_error, "unknown restrictor mode: " << args[0]);
    }
  }
}

DpadRestrictorModifier::DpadRestrictorModifier(Mode mode) :
  m_mode(mode),
  m_last_unpressed_axis(XBOX_AXIS_DPAD_X)
{
}

void
DpadRestrictorModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  switch(m_mode)
  {
    case kRestrictFourWay:
      if (msg.get_axis(XBOX_AXIS_DPAD_X) && msg.get_axis(XBOX_AXIS_DPAD_Y))
      { 
        // a diagonal was pressed, thus we reset the axis that wasn't
        // pressed the last time the dpad was touched
        msg.set_axis(m_last_unpressed_axis, 0);
      }
      else if (msg.get_axis(XBOX_AXIS_DPAD_X))
      {
        m_last_unpressed_axis = XBOX_AXIS_DPAD_Y;
      }
      else if (msg.get_axis(XBOX_AXIS_DPAD_Y))
      {
        m_last_unpressed_axis = XBOX_AXIS_DPAD_X;
      }
      break;

    case kRestrictXAxis:
      msg.set_axis(XBOX_AXIS_DPAD_Y, 0);
      break;

    case kRestrictYAxis:
      msg.set_axis(XBOX_AXIS_DPAD_X, 0);
      break;
  }
}

std::string
DpadRestrictorModifier::str() const
{
  switch(m_mode)
  {
    case kRestrictFourWay: return "dpad-restrictor:four-way";
    case kRestrictXAxis:   return "dpad-restrictor:x-axis";
    case kRestrictYAxis:   return "dpad-restrictor:y-axis";
    default: assert(!"never reached");
  }
}

/* EOF */
