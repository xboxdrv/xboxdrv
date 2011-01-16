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

class AutofireMapping 
{
public:
  static AutofireMapping from_string(const std::string& lhs, const std::string& rhs);

public:  
  AutofireMapping(XboxButton button, int frequency);

  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kAutofirePriority; };

public:
  XboxButton m_button;
  int m_frequency;

  int m_button_timer;
};

class AutofireModifier : public Modifier
{
private:
  std::vector<AutofireMapping> m_autofire_map;
  std::vector<int> m_button_timer;

public:
  AutofireModifier(const std::vector<AutofireMapping>& autofire_map);
    
  void update(int msec_delta, XboxGenericMsg& msg);

  Modifier::Priority get_priority() const { return Modifier::kAutofirePriority; };
};

#endif

/* EOF */
