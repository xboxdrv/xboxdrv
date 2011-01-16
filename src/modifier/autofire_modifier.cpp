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

#include "autofire_modifier.hpp"

#include <boost/lexical_cast.hpp>

AutofireModifier*
AutofireModifier::from_string(const std::string& lhs, const std::string& rhs)
{
  /* Format of str: A={ON-DELAY}[:{OFF-DELAY}]
     Examples: A=10 or A=10:50 
     if OFF-DELAY == nil then ON-DELAY = OFF-DELAY 
  */
  return new AutofireModifier(string2btn(lhs), boost::lexical_cast<int>(rhs));
}

AutofireModifier::AutofireModifier(XboxButton button, int frequency) :
  m_button(button),
  m_frequency(frequency),
  m_button_timer(0)
{
}

void
AutofireModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  if (get_button(msg, m_button))
  {
    m_button_timer += msec_delta;

    if (m_button_timer > m_frequency)
    {
      set_button(msg, m_button, 1);
      m_button_timer = 0; // FIXME: we ignoring the passed time
    }
    else if (m_button_timer > m_frequency/2)
    {
      set_button(msg, m_button, 0);
    }
    else
    {
      set_button(msg, m_button, 1);
    }
  }
  else
  {
    m_button_timer = 0;
  } 
}

/* EOF */
