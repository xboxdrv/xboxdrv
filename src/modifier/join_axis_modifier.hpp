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

#ifndef HEADER_XBOXDRV_MODIFIER_JOIN_AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_JOIN_AXIS_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"
#include "abs_port.hpp"

class JoinAxisModifier : public Modifier
{
public: 
  static JoinAxisModifier* from_string(const std::vector<std::string>& args);

private:
  AbsPortIn  m_lhs;
  AbsPortIn  m_rhs;
  AbsPortOut m_out;

public:
  JoinAxisModifier(const std::string& lhs, const std::string& rhs, const std::string& out);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  JoinAxisModifier(const JoinAxisModifier&);
  JoinAxisModifier& operator=(const JoinAxisModifier&);
};

#endif

/* EOF */
