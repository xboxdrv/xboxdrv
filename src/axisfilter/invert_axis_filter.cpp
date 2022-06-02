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

#include "invert_axis_filter.hpp"

namespace xboxdrv {

int
InvertAxisFilter::filter(int value, int min, int max)
{
  int center = (max + min + 1)/2; // FIXME: '+1' is kind of a hack to
                                  // get the center at 0 for the
                                  // [-32768, 32767] case
  if (value < center)
  {
    return (max - center) * (value - center) / (min - center) + center;
  }
  else if (value > center)
  {
    return (min - center) * (value - center) / (max - center) + center;
  }
  else
  {
    return value;
  }
}

std::string
InvertAxisFilter::str() const
{
  return "invert";
}

} // namespace xboxdrv

/* EOF */
