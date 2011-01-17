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

#ifndef HEADER_XBOXDRV_MODIFIER_RELATIVEAXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_RELATIVEAXIS_MODIFIER_HPP

#include "modifier.hpp"

class RelativeAxisModifier : public Modifier
{
public:
  static RelativeAxisModifier* from_string(const std::string& lhs, const std::string& rhs);
  static RelativeAxisModifier* from_string(const std::vector<std::string>& args);

public:
  RelativeAxisModifier();

  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kRelativeAxisPriority; };

public:
  XboxAxis m_axis;
  int      m_speed;

  int m_axis_state;
};

#endif

/* EOF */
