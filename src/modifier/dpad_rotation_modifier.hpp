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

#ifndef HEADER_XBOXDRV_MODIFIER_DPAD_ROTATION_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_DPAD_ROTATION_MODIFIER_HPP

#include <vector>

#include "key_port.hpp"
#include "modifier.hpp"

class ControllerMessage;

class DpadRotationModifier : public Modifier
{
public:
  static DpadRotationModifier* from_string(const std::vector<std::string>& args);
  static DpadRotationModifier* from_string(const std::string& value);

private:
  int  m_dpad_rotation;

  KeyPortIn m_dpad_up;
  KeyPortIn m_dpad_down;
  KeyPortIn m_dpad_left;
  KeyPortIn m_dpad_right;

  KeyPortOut m_dpad_up_out;
  KeyPortOut m_dpad_down_out;
  KeyPortOut m_dpad_left_out;
  KeyPortOut m_dpad_right_out;

public:
  DpadRotationModifier(int dpad_rotation);

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  DpadRotationModifier(const DpadRotationModifier&);
  DpadRotationModifier& operator=(const DpadRotationModifier&);
};

#endif

/* EOF */
