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

#ifndef HEADER_XBOXDRV_MODIFIER_AUTOFIRE_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_AUTOFIRE_MODIFIER_HPP

#include <vector>

#include "modifier.hpp"

class AutofireModifier : public Modifier
{
public:
  static AutofireModifier* from_string(const std::string& lhs, const std::string& rhs);

  /** input: [ BUTTON, FREQUENCY ] */
  static AutofireModifier* from_string(const std::vector<std::string>& args);

public:  
  AutofireModifier(XboxButton button, int frequency);

  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kAutofirePriority; };

public:
  XboxButton m_button;
  int m_frequency;

  int m_button_timer;
};

#endif

/* EOF */
