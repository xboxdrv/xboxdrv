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

#include "button_map_modifier.hpp"

ButtonMapModifier::ButtonMapModifier(const std::vector<ButtonMapping>& buttonmap) :
  m_buttonmap(buttonmap)
{
}

void
ButtonMapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<ButtonMapping>::const_iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
    set_button(newmsg, i->lhs, 0);

  for(std::vector<ButtonMapping>::const_iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
    set_button(newmsg, i->rhs, get_button(msg, i->lhs) || get_button(newmsg, i->rhs));

  msg = newmsg;  
}

/* EOF */
