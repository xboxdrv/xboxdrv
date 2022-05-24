/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2020 Ingo Ruhnke <grumbel@gmail.com>
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

#include "util/math.hpp"

float to_float_no_range_check(int value, int min, int max)
{
  // FIXME: '+1' is kind of a hack to
  // get the center at 0 for the
  // [-32768, 32767] case
  int center = (max + min + 1)/2;

  if (value < center)
  {
    return static_cast<float>(value - center) / static_cast<float>(center - min);
  }
  else // (value >= center)
  {
    return static_cast<float>(value - center) / static_cast<float>(max - center);
  }
}

float to_float(int value, int min, int max)
{
  return std::clamp(to_float_no_range_check(value, min, max),
                    -1.0f, 1.0f);
}

int from_float(float value, int min, int max)
{
  return static_cast<int>((value + 1.0f) / 2.0f * static_cast<float>(max - min)) + min;
}

/* EOF */
