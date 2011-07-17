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

#include "rotate_axis_modifier.hpp"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <math.h>

RotateAxisModifier*
RotateAxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 3 && args.size() != 4)
  {
    throw std::runtime_error("RotateAxisModifier requires three or four arguments");
  }
  else
  {
    return new RotateAxisModifier(string2axis(args[0]),
                                  string2axis(args[1]),
                                  boost::lexical_cast<float>(args[2]) * M_PI / 180.0f,
                                  args.size() == 3 ? false : boost::lexical_cast<bool>(args[3]));
  }
}

RotateAxisModifier::RotateAxisModifier(XboxAxis xaxis, XboxAxis yaxis, float angle, bool mirror) :
  m_xaxis(xaxis),
  m_yaxis(yaxis),
  m_angle(angle),
  m_mirror(mirror)
{
}

void
RotateAxisModifier::update(int msec_delta, ControllerMessage& msg)
{
  float x = msg.get_axis_float(m_xaxis);
  float y = msg.get_axis_float(m_yaxis);

  if (m_mirror)
  {
    x = -x;
  }

  float length = sqrtf(x*x + y*y);
  float angle = atan2f(y, x) + m_angle;

  msg.set_axis_float(m_xaxis, cos(angle) * length);
  msg.set_axis_float(m_yaxis, sin(angle) * length);
}

std::string
RotateAxisModifier::str() const
{
  std::ostringstream out;
  out << "rotate:" << m_xaxis << "=" << m_yaxis << ":" << (m_angle/180.0f*M_PI);
  return out.str();
}

/* EOF */
