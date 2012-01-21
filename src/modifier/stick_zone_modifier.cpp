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
    return new StickZoneModifier(args[0], args[1], args[2],
                                 boost::lexical_cast<float>(args[3]),
                                 boost::lexical_cast<float>(args[4]));
  }
}

StickZoneModifier::StickZoneModifier(const std::string& x_axis, const std::string& y_axis, 
                                     const std::string& button, 
                                     float range_start, float range_end) :

  m_x_axis_str(x_axis),
  m_y_axis_str(y_axis),
  m_button_str(button),
  m_x_axis(-1),
  m_y_axis(-1),
  m_button(-1),
  m_range_start(range_start),
  m_range_end(range_end)
{
}

void
StickZoneModifier::init(ControllerMessageDescriptor& desc)
{
  m_x_axis = desc.abs().get(m_x_axis_str);
  m_y_axis = desc.abs().get(m_y_axis_str);
  m_button = desc.key().get(m_button_str);
}

void
StickZoneModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  float x = msg.get_abs_float(m_x_axis);
  float y = msg.get_abs_float(m_y_axis);
  float r = sqrtf(x*x + y*y);

  if (r > 1.0f)
  {
    r = 1.0f;
  }

  if (m_range_start <= r && r <= m_range_end)
  {
    msg.set_key(m_button, true);
  }
  else
  {
    msg.set_key(m_button, false);
  }
}

std::string
StickZoneModifier::str() const
{
  std::ostringstream os;
  //BROKEN:os << "stickzone:" << axis2string(m_x_axis) << ":" << axis2string(m_y_axis) << ":" << btn2string(m_button);
  return os.str();
}

/* EOF */
