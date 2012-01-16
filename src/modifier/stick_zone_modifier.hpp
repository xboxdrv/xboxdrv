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

#ifndef HEADER_XBOXDRV_MODIFIER_STICK_ZONE_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_STICK_ZONE_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"

class StickZoneModifier : public Modifier
{
public:
  static StickZoneModifier* from_string(const std::vector<std::string>& args);

private:
  const std::string m_x_axis_str;
  const std::string m_y_axis_str;
  const std::string m_button_str;

  int m_x_axis;
  int m_y_axis;
  int m_button;

  float m_range_start;
  float m_range_end;

public:
  StickZoneModifier(const std::string& x_axis, const std::string& y_axis, const std::string& button,
                    float range_start, float range_end);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg);

  std::string str() const;

private:
  StickZoneModifier(const StickZoneModifier&);
  StickZoneModifier& operator=(const StickZoneModifier&);
};

#endif

/* EOF */
