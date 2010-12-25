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

#ifndef HEADER_XBOXDRV_AXIS_FILTER_HPP
#define HEADER_XBOXDRV_AXIS_FILTER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

class AxisFilter;

typedef boost::shared_ptr<AxisFilter> AxisFilterPtr;

class AxisFilter
{
public:
  static AxisFilterPtr from_string(const std::string& str);

public:
  AxisFilter() {}
  virtual ~AxisFilter() {}

  virtual bool filter(bool old_value, bool value) =0;
};

#endif

/* EOF */
