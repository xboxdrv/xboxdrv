/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_MODIFIER_COMPAT_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_COMPAT_MODIFIER_HPP

#include "modifier.hpp"

class CompatModifier : public Modifier
{
private:
  bool m_dpad;
  int m_dpad_x;
  int m_dpad_y;
  int m_dpad_up;
  int m_dpad_down;
  int m_dpad_left;
  int m_dpad_right;

  bool m_trigger;
  int m_abs_trigger;
  int m_lt;
  int m_rt;

public:
  CompatModifier();

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc);

  std::string str() const;

private:
  CompatModifier(const CompatModifier&);
  CompatModifier& operator=(const CompatModifier&);
};

#endif

/* EOF */
