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

#ifndef HEADER_XBOXDRV_MODIFIER_FOUR_WAY_RESTRICTOR_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_FOUR_WAY_RESTRICTOR_MODIFIER_HPP

#include "modifier.hpp"

class FourWayRestrictorModifier : public Modifier
{
public:
  static FourWayRestrictorModifier* from_string(const std::vector<std::string>& args);

public:
  FourWayRestrictorModifier(XboxAxis xaxis, XboxAxis yaxis);

  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kFourWayRestrictorPriority; };

  std::string str() const;

private:
  const XboxAxis m_xaxis;
  const XboxAxis m_yaxis;
};

#endif

/* EOF */
