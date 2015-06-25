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

#include "square_axis_modifier.hpp"

#include <math.h>
#include <sstream>

#include "controller_message.hpp"
#include "helper.hpp"

namespace {

void squarify_axis(float& x_inout, float& y_inout)
{
  if (x_inout != 0 || y_inout != 0)
  {
    // Convert values to float
    float x = x_inout;
    float y = y_inout;

    // Transform values to square range
    float l = sqrtf(x*x + y*y);
    float v = fabsf((fabsf(x) > fabsf(y)) ? l/x : l/y);
    x *= v;
    y *= v;

    // Convert values to int
    x_inout = Math::clamp(-1.0f, x, 1.0f);
    y_inout = Math::clamp(-1.0f, y, 1.0f);
  }
}

} // namespace


SquareAxisModifier*
SquareAxisModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() == 2)
  {
    return new SquareAxisModifier(args[0], args[1], args[0], args[1]);
  }
  else if (args.size() == 4)
  {
    return new SquareAxisModifier(args[0], args[1], args[2], args[3]);
  }
  else
  {
    throw std::runtime_error("SquareAxisModifier requires two or four arguments");
  }
}

SquareAxisModifier::SquareAxisModifier(const std::string& x_axis_in,  const std::string& y_axis_in,
                                       const std::string& x_axis_out, const std::string& y_axis_out) :
  m_xaxis_in(x_axis_in),
  m_yaxis_in(y_axis_in),
  m_xaxis_out(x_axis_out),
  m_yaxis_out(y_axis_out)
{
}

void
SquareAxisModifier::init(ControllerMessageDescriptor& desc)
{
  m_xaxis_in.init(desc);
  m_yaxis_in.init(desc);

  m_xaxis_out.init(desc);
  m_yaxis_out.init(desc);
}

void
SquareAxisModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  float x = m_xaxis_in.get_float(msg);
  float y = m_yaxis_in.get_float(msg);

  squarify_axis(x, y);

  m_xaxis_out.set_float(msg, x);
  m_yaxis_out.set_float(msg, y);
}

std::string
SquareAxisModifier::str() const
{
  std::ostringstream out;
  out << "square:"
      << m_xaxis_in.str() << ":"
      << m_yaxis_in.str() << ":"
      << m_xaxis_out.str() << ":"
      << m_yaxis_out.str();
  return out.str();
}

/* EOF */
