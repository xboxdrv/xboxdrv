/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_AXIS_EVENT_HPP
#define HEADER_XBOXDRV_AXIS_EVENT_HPP

#include <string>

struct AxisEvent
{
  static AxisEvent invalid() { return create(-1, -1); }
  static AxisEvent create(int type, int code, int fuzz = 0, int flat = 0);
  static AxisEvent from_string(const std::string& str);

  /** EV_KEY, EV_ABS, EV_REL */
  int type;

  /** BTN_A, REL_X, ABS_X, ... */
  int code;

  union {
    struct {
      int   repeat;
      float value;
    } rel;

    struct {
      int fuzz;
      int flat;
    } abs;

    struct {
      int secondary_code;
      int threshold;
    } key;
  };
};

#endif

/* EOF */
