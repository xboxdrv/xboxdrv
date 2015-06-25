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

#ifndef HEADER_XBOXDRV_MODIFIER_SPLIT_AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_SPLIT_AXIS_MODIFIER_HPP

#include <vector>

#include "abs_port.hpp"
#include "modifier.hpp"

class SplitAxisModifier : public Modifier
{
public:
  static SplitAxisModifier* from_string(const std::vector<std::string>& args);

private:
  AbsPortIn m_axis;
  AbsPortOut m_out_lhs;
  AbsPortOut m_out_rhs;

public:
  SplitAxisModifier(const std::string& axis, const std::string& out_lhs, const std::string& out_rhs);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  SplitAxisModifier(const SplitAxisModifier&);
  SplitAxisModifier& operator=(const SplitAxisModifier&);
};

#endif

/* EOF */
