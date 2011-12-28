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

#include "button_combination.hpp"

#include <boost/tokenizer.hpp>
#include <algorithm>

#include "xboxmsg.hpp"

ButtonCombination
ButtonCombination::from_string(const std::string& str)
{
  ButtonCombination comp;

  if (str == "void")
  {
    return comp;
  }
  else
  {
    boost::tokenizer<boost::char_separator<char> > 
      btn_tokens(str, boost::char_separator<char>("+", "", boost::keep_empty_tokens));

    std::transform(btn_tokens.begin(), btn_tokens.end(),
                   std::back_inserter(comp.m_buttons), string2btn);

    std::sort(comp.m_buttons.begin(), comp.m_buttons.end());

    return comp;
  }
}

ButtonCombination::ButtonCombination() :
  m_buttons()
{
}

ButtonCombination::ButtonCombination(XboxButton button) :
  m_buttons(1)
{
  m_buttons[0] = button;
}

ButtonCombination::ButtonCombination(const std::vector<XboxButton>& buttons) :
  m_buttons(buttons)
{
  std::sort(m_buttons.begin(), m_buttons.end());
}

bool
ButtonCombination::has_button(XboxButton button) const
{
  return std::find(m_buttons.begin(), m_buttons.end(), button) != m_buttons.end();
}

bool
ButtonCombination::is_subset_of(const ButtonCombination& rhs) const
{
  if (m_buttons.empty())
  {
    return false;
  }
  else
  {
    for(Buttons::const_iterator i = m_buttons.begin(); i != m_buttons.end(); ++i)
    {
      if (!rhs.has_button(*i))
      {
        return false;
      }
    }
    return true;
  }
}

int
ButtonCombination::size() const
{
  return static_cast<int>(m_buttons.size());
}

bool
ButtonCombination::match(const std::bitset<XBOX_BTN_MAX>& button_state) const
{
  for(std::vector<XboxButton>::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
  {
    if (!button_state[*btn])
    {
      return false;
    }
  }
  return true;
}

void
ButtonCombination::print(std::ostream& os) const
{
  for(std::vector<XboxButton>::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
  {
    os << btn2string(*btn);
    if (btn != m_buttons.end()-1)
      os << "+";
  }
}

bool
ButtonCombination::empty() const
{
  return m_buttons.empty();
}

bool
ButtonCombination::operator==(const ButtonCombination& rhs) const
{
  if (m_buttons.size() != rhs.m_buttons.size())
  {
    return false;
  }
  else
  {
    return std::equal(m_buttons.begin(), m_buttons.end(),
                      rhs.m_buttons.begin());
  }
}

std::ostream& operator<<(std::ostream& os, const ButtonCombination& buttons)
{
  buttons.print(os);
  return os;
}

/* EOF */
