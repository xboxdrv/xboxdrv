/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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
    return new Acc2AxisModifier(args[0], args[1], args[2],
                                args[3], args[4]);
  }
}

Acc2AxisModifier::Acc2AxisModifier(const std::string& acc_x, const std::string& acc_y, const std::string& acc_z,
                                   const std::string& axis_x, const std::string& axis_y) :
  m_acc_x_str(acc_x),
  m_acc_y_str(acc_y),
  m_acc_z_str(acc_z),
  m_axis_x_str(axis_x),
  m_axis_y_str(axis_y),
  m_acc_x(-1),
  m_acc_y(-1),
  m_acc_z(-1),
  m_axis_x(-1),
  m_axis_y(-1)
{
}

void
Acc2AxisModifier::init(ControllerMessageDescriptor& desc)
{
  m_acc_x = desc.abs().get(m_acc_x_str);
  m_acc_y = desc.abs().get(m_acc_y_str);
  m_acc_z = desc.abs().get(m_acc_z_str);

  m_axis_x = desc.abs().getput(m_axis_x_str);
  m_axis_y = desc.abs().getput(m_axis_y_str);
}

void
Acc2AxisModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  float ax = msg.get_abs_float(m_acc_x);
  float ay = msg.get_abs_float(m_acc_y);
  float az = msg.get_abs_float(m_acc_z);

  float rx = (atan2f(az, ax) - static_cast<float>(M_PI)/2.0f) * -1.0f;
  float ry = (atan2f(az, ay) - static_cast<float>(M_PI)/2.0f) * -1.0f;

  // FIXME: to simplistic, need a better way to normalize the angles
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

  msg.set_abs_float(m_axis_x, rx);
  msg.set_abs_float(m_axis_y, ry);
}

std::string
Acc2AxisModifier::str() const
{
  return "acc2axis";
}

/* EOF */
