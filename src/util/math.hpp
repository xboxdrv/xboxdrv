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

#ifndef HEADER_XBOXDRV_UTIL_MATH_HPP
#define HEADER_XBOXDRV_UTIL_MATH_HPP

#include <algorithm>
#include <assert.h>

/** converts the arbitary range to [-1,1] */
float to_float(int value, int min, int max);
float to_float_no_range_check(int value, int min, int max);

/** converts the range [-1,1] to [min,max] */
int from_float(float value, int min, int max);

#endif

/* EOF */
