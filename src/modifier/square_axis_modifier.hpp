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

#ifndef HEADER_XBOXDRV_MODIFIER_SQUARE_AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_SQUARE_AXIS_MODIFIER_HPP

#include <vector>

#include "abs_port.hpp"
#include "modifier.hpp"

class SquareAxisModifier : public Modifier
{
public:
  static SquareAxisModifier* from_string(const std::vector<std::string>& args);

public:
  SquareAxisModifier(const std::string& x_axis_in,  const std::string& y_axis_in,
                     const std::string& x_axis_out, const std::string& y_axis_out);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  AbsPortIn m_xaxis_in;
  AbsPortIn m_yaxis_in;

  AbsPortOut m_xaxis_out;
  AbsPortOut m_yaxis_out;
};

#endif

/* EOF */
