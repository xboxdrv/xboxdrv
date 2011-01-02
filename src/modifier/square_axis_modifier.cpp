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

#include <math.h>

#include "helper.hpp"
#include "square_axis_modifier.hpp"

namespace {

void squarify_axis_(int16_t& x_inout, int16_t& y_inout)
{
  if (x_inout != 0 || y_inout != 0)
  {
    // Convert values to float
    float x = (x_inout < 0) ? static_cast<float>(x_inout) / 32768.0f : static_cast<float>(x_inout) / 32767.0f;
    float y = (y_inout < 0) ? static_cast<float>(y_inout) / 32768.0f : static_cast<float>(y_inout) / 32767.0f;

    // Transform values to square range
    float l = sqrtf(x*x + y*y);
    float v = fabsf((fabsf(x) > fabsf(y)) ? l/x : l/y);
    x *= v;
    y *= v;

    // Convert values to int16_t
    x_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((x < 0) ? x * 32768 : x * 32767), 32767));
    y_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((y < 0) ? y * 32768 : y * 32767), 32767));
  }
}

// Little hack to allow access to bitfield via reference
#define squarify_axis(x, y)                     \
  {                                             \
    int16_t x_ = x;                             \
    int16_t y_ = y;                             \
    squarify_axis_(x_, y_);                     \
    x = x_;                                     \
    y = y_;                                     \
  }

} // namespace


SquareAxisModifier::SquareAxisModifier()
{
}

void
SquareAxisModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  switch (msg.type)
  {
    case XBOX_MSG_XBOX:
      squarify_axis(msg.xbox.x1, msg.xbox.y1);
      squarify_axis(msg.xbox.x2, msg.xbox.y2);
      break;

    case XBOX_MSG_XBOX360:
      squarify_axis(msg.xbox360.x1, msg.xbox360.y1);
      squarify_axis(msg.xbox360.x2, msg.xbox360.y2);
      break;
  }
}

/* EOF */
