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

#include "ir2axis_modifier.hpp"

#include <stdexcept>

#include "raise_exception.hpp"

IR2AxisModifier*
IR2AxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 2)
  {
    raise_exception(std::runtime_error, "two axis required as argument");
  }
  else
  {
    return new IR2AxisModifier(string2axis(args[0]),
                               string2axis(args[1]));
  }
}

IR2AxisModifier::IR2AxisModifier(XboxAxis axis_x, XboxAxis axis_y) :
  m_axis_x(axis_x),
  m_axis_y(axis_y)
{
}

void
IR2AxisModifier::update(int msec_delta, ControllerMessage& msg)
{
  // find center of two biggest points, return that as axis values

  if (msg.get_axis(WIIMOTE_IR_SIZE) >= 0)
  {
    msg.set_axis_float(m_axis_x, msg.get_axis_float(WIIMOTE_IR_X));
    msg.set_axis_float(m_axis_y, msg.get_axis_float(WIIMOTE_IR_Y));
  }
}

std::string
IR2AxisModifier::str() const
{
  return "ir2axis";
}

/* EOF */
