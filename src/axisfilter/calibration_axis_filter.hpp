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

#ifndef HEADER_XBOXDRV_AXISFILTER_CALIBRATION_AXIS_FILTER_HPP
#define HEADER_XBOXDRV_AXISFILTER_CALIBRATION_AXIS_FILTER_HPP

#include "axis_filter.hpp"

class CalibrationAxisFilter : public AxisFilter
{
public:
  static CalibrationAxisFilter* from_string(const std::string& str);

public:
  CalibrationAxisFilter(int min, int center, int max);

  int filter(int value, int min, int max) override;
  std::string str() const override;

private:
  int m_min;
  int m_center;
  int m_max;
};

#endif

/* EOF */
