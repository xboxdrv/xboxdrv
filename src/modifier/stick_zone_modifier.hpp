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

#ifndef HEADER_XBOXDRV_MODIFIER_STICK_ZONE_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_STICK_ZONE_MODIFIER_HPP

#include <vector>

#include "abs_port.hpp"
#include "key_port.hpp"
#include "modifier.hpp"

namespace xboxdrv {

class StickZoneModifier : public Modifier
{
public:
  static StickZoneModifier* from_string(const std::vector<std::string>& args);

private:
  AbsPortIn m_x_axis;
  AbsPortIn m_y_axis;
  KeyPortOut m_button;

  float m_range_start;
  float m_range_end;

public:
  StickZoneModifier(const std::string& x_axis, const std::string& y_axis, const std::string& button,
                    float range_start, float range_end);

  void init(ControllerMessageDescriptor& desc) override;
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc) override;

  std::string str() const override;

private:
  StickZoneModifier(const StickZoneModifier&);
  StickZoneModifier& operator=(const StickZoneModifier&);
};

} // namespace xboxdrv

#endif

/* EOF */
