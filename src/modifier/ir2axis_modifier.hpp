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

#ifndef HEADER_XBOXDRV_MODIFIER_IR2AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_IR2AXIS_MODIFIER_HPP

#include "modifier.hpp"

#include <vector>

#include "xboxmsg.hpp"

class IR2AxisModifier : public Modifier
{
public:
  static IR2AxisModifier* from_string(const std::vector<std::string>& args);

private:
  const std::string m_xaxis_str;
  const std::string m_yaxis_str;

  int m_xaxis;
  int m_yaxis;

public:
  IR2AxisModifier(const std::string& xaxis, const std::string& yaxis);

  void init(ControllerMessageDescriptor& desc) override;
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc) override;
  std::string str() const override;

private:
  IR2AxisModifier(const IR2AxisModifier&);
  IR2AxisModifier& operator=(const IR2AxisModifier&);
};

#endif

/* EOF */
