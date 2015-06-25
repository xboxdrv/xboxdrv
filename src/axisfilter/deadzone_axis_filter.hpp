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

#ifndef HEADER_XBOXDRV_AXISFILTER_DEADZONE_AXIS_FILTER_HPP
#define HEADER_XBOXDRV_AXISFILTER_DEADZONE_AXIS_FILTER_HPP

#include "axis_filter.hpp"

class DeadzoneAxisFilter : public AxisFilter
{
public:
  static DeadzoneAxisFilter* from_string(const std::string& str);

public:
  DeadzoneAxisFilter(int min_deadzone, int max_deathzone, bool smooth);

  int filter(int value, int min, int max);
  std::string str() const;

private:
  int m_min_deadzone;
  int m_max_deadzone;
  bool m_smooth;
};

#endif

/* EOF */
