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

#ifndef HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"

class AxisMapping 
{
public:
  static AxisMapping from_string(const std::string& lhs, const std::string& rhs);

public:
  XboxAxis m_lhs;
  XboxAxis m_rhs;
  bool     m_invert;
  std::vector<AxisFilterPtr> m_filters;
};

class AxismapModifier : public Modifier
{
private:
  std::vector<AxisMapping> m_axismap;

public:
  AxismapModifier(const std::vector<AxisMapping>& axismap);

  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kAxismapPriority; };

private:
  AxismapModifier(const AxismapModifier&);
  AxismapModifier& operator=(const AxismapModifier&);
};

#endif

/* EOF */
