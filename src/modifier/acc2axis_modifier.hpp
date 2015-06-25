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

#ifndef HEADER_XBOXDRV_MODIFIER_ACC2AXIS_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_ACC2AXIS_MODIFIER_HPP

#include "modifier.hpp"

#include <vector>

class Acc2AxisModifier : public Modifier
{
public:
  static Acc2AxisModifier* from_string(const std::vector<std::string>& args);

private:
  const std::string m_acc_x_str;
  const std::string m_acc_y_str;
  const std::string m_acc_z_str;

  const std::string m_axis_x_str;
  const std::string m_axis_y_str;

  int m_acc_x;
  int m_acc_y;
  int m_acc_z;

  int m_axis_x;
  int m_axis_y;

public:
  Acc2AxisModifier(const std::string& acc_x, const std::string& acc_y, const std::string& acc_z,
                   const std::string& axis_x, const std::string& axis_y);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);
  std::string str() const;

private:
  Acc2AxisModifier(const Acc2AxisModifier&);
  Acc2AxisModifier& operator=(const Acc2AxisModifier&);
};

#endif

/* EOF */
