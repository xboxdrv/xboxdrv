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

#ifndef HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP

#include "axis_filter.hpp"
#include "modifier.hpp"

struct AxisMapping
{
  static AxisMapping from_string(const std::string& lhs, const std::string& rhs);

  XboxAxis lhs;
  XboxAxis rhs;
  bool     invert;
  std::vector<AxisFilterPtr> filters;

  AxisMapping() :
    lhs(XBOX_AXIS_UNKNOWN),
    rhs(XBOX_AXIS_UNKNOWN),
    invert(false),
    filters()
  {}
};

class AxismapModifier : public Modifier
{
public:
  AxismapModifier();

  void update(int msec_delta, XboxGenericMsg& msg);

  void add(const AxisMapping& mapping);
  void add_filter(XboxAxis axis, AxisFilterPtr filter);

  std::string str() const;

  bool empty() const { return m_axismap.empty(); }

public:
  std::vector<AxisMapping> m_axismap;
};

#endif

/* EOF */
