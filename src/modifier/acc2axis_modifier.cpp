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

#include "acc2axis_modifier.hpp"

#include <stdexcept>
#include <math.h>

#include "helper.hpp"
#include "log.hpp"
#include "raise_exception.hpp"

Acc2AxisModifier*
Acc2AxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 5)
  {
    raise_exception(std::runtime_error, "acc2axis requires five arguments");
  }
  else
  {
    return new Acc2AxisModifier(string2axis(args[0]),
                                string2axis(args[1]),
                                string2axis(args[2]),
                                string2axis(args[3]),
                                string2axis(args[4]));
  }
}

Acc2AxisModifier::Acc2AxisModifier(XboxAxis acc_x, XboxAxis acc_y, XboxAxis acc_z,
                                   XboxAxis axis_x, XboxAxis axis_y) :
  m_acc_x(acc_x),
  m_acc_y(acc_y),
  m_acc_z(acc_z),
  m_axis_x(axis_x),
  m_axis_y(axis_y)
{
}

void
Acc2AxisModifier::update(int msec_delta, ControllerMessage& msg)
{
  float ax = msg.get_axis_float(m_acc_x);
  float ay = msg.get_axis_float(m_acc_y);
  float az = msg.get_axis_float(m_acc_z);

  float rx = (atan2f(az, ax) - static_cast<float>(M_PI)/2.0f) * -1.0f;
  float ry = (atan2f(az, ay) - static_cast<float>(M_PI)/2.0f) * -1.0f;

  // normalize the range from [-pi/2, 3/2pi] to [-pi,pi]
  if (rx > static_cast<float>(M_PI))
    rx -= static_cast<float>(M_PI*2.0);

  if (ry > static_cast<float>(M_PI))
    ry -= static_cast<float>(M_PI*2.0);

  ry *= -1.0f;

  // full range is M_PI, but we use M_PI/2 as beyond that point
  // precision is lacking
  rx = static_cast<float>(rx/(M_PI/2.0));
  ry = static_cast<float>(ry/(M_PI/2.0));

  rx = Math::clamp(-1.0f, rx, 1.0f);
  ry = Math::clamp(-1.0f, ry, 1.0f);

  //log_tmp("Rot: " /*<< ax << " " << ay << " " << az << " -- " */<< rx << " " << ry);

  msg.set_axis_float(m_axis_x, rx);
  msg.set_axis_float(m_axis_y, ry);
}

std::string
Acc2AxisModifier::str() const
{
  return "acc2axis";
}

/* EOF */
