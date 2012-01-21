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

#ifndef HEADER_XBOXDRV_MODIFIER_SQUARE_AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_SQUARE_AXIS_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"

class SquareAxisModifier : public Modifier
{
public:
  static SquareAxisModifier* from_string(const std::vector<std::string>& args);

public:
  SquareAxisModifier(const std::string& x_axis, const std::string& y_axis);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  const std::string m_xaxis_str;
  const std::string m_yaxis_str;

  int m_xaxis;
  int m_yaxis;
};

#endif

/* EOF */
