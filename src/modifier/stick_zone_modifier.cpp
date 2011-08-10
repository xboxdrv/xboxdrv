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

#include "modifier/stick_zone_modifier.hpp"

#include <boost/lexical_cast.hpp>
#include <math.h>
#include <sstream>
#include <stdexcept>

#include "xboxmsg.hpp"
#include "raise_exception.hpp"

StickZoneModifier*
StickZoneModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 5)
  {
    raise_exception(std::runtime_error, "StickZoneModifier requires five arguments");
  }
  else
  {
    return new StickZoneModifier(string2axis(args[0]),
                                 string2axis(args[1]),
                                 string2btn(args[2]),
                                 boost::lexical_cast<float>(args[3]),
                                 boost::lexical_cast<float>(args[4]));
  }
}

StickZoneModifier::StickZoneModifier(XboxAxis x_axis, XboxAxis y_axis, XboxButton button, 
                                     float range_start, float range_end) :
  m_x_axis(x_axis),
  m_y_axis(y_axis),
  m_button(button),
  m_range_start(range_start),
  m_range_end(range_end)
{
}

void
StickZoneModifier::update(int msec_delta, ControllerMessage& msg)
{
  float x = msg.get_axis_float(m_x_axis);
  float y = msg.get_axis_float(m_y_axis);
  float r = sqrtf(x*x + y*y);

  if (r > 1.0f)
  {
    r = 1.0f;
  }

  if (m_range_start <= r && r <= m_range_end)
  {
    msg.set_button(m_button, true);
  }
  else
  {
    msg.set_button(m_button, false);
  }
}

std::string
StickZoneModifier::str() const
{
  std::ostringstream os;
  os << "stickzone:" << axis2string(m_x_axis) << ":" << axis2string(m_y_axis) << ":" << btn2string(m_button);
  return os.str();
}

/* EOF */
