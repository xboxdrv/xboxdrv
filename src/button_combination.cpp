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

#include "controller_message_descriptor.hpp"

ButtonCombination
ButtonCombination::from_string(const std::string& str)
{
  if (str == "void")
  {
    return ButtonCombination();
  }
  else
  {
    boost::tokenizer<boost::char_separator<char> > 
      btn_tokens(str, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
    return ButtonCombination(std::vector<std::string>(btn_tokens.begin(), btn_tokens.end()));
  }
}

ButtonCombination::ButtonCombination() :
  m_buttons_str(),
  m_buttons()
{
}

ButtonCombination::ButtonCombination(const std::string& button) :
  m_buttons_str(),
  m_buttons()
{
  m_buttons_str.push_back(button);
}

ButtonCombination::ButtonCombination(const std::vector<std::string>& buttons) :
  m_buttons_str(buttons),
  m_buttons()
{
  std::sort(m_buttons_str.begin(), m_buttons_str.end());
}

void
ButtonCombination::init(const ControllerMessageDescriptor& desc)
{
  m_buttons.clear();
  for(ButtonsStr::const_iterator it = m_buttons_str.begin(); it != m_buttons_str.end(); ++it)
  {
    m_buttons.push_back(desc.key().get(*it));
  }
  std::sort(m_buttons.begin(), m_buttons.end());
}

bool
ButtonCombination::has_button(const std::string& button) const
{
  return std::find(m_buttons_str.begin(), m_buttons_str.end(), button) != m_buttons_str.end();
}

bool
ButtonCombination::is_subset_of(const ButtonCombination& rhs) const
{
  if (m_buttons_str.empty())
  {
    return false;
  }
  else
  {
    for(ButtonsStr::const_iterator i = m_buttons_str.begin(); i != m_buttons_str.end(); ++i)
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
  return static_cast<int>(m_buttons_str.size());
}

bool
ButtonCombination::match(const std::bitset<256>& button_state) const
{
  //std::cout << "ButtonCombination::match: " << m_buttons.size() << std::endl;
  for(Buttons::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
  {
    //std::cout << " ... " << *btn << " " << button_state[*btn] << std::endl;
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
  for(ButtonsStr::const_iterator btn = m_buttons_str.begin(); btn != m_buttons_str.end(); ++btn)
  {
    os << *btn;
    if (btn != m_buttons_str.end()-1)
      os << "+";
  }
  os << " ";

  for(Buttons::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
  {
    os << *btn;
    if (btn != m_buttons.end()-1)
      os << "+";
  }
}

bool
ButtonCombination::empty() const
{
  return m_buttons_str.empty();
}

bool
ButtonCombination::operator==(const ButtonCombination& rhs) const
{
  if (m_buttons_str.size() != rhs.m_buttons_str.size())
  {
    return false;
  }
  else
  {
    return std::equal(m_buttons_str.begin(), m_buttons_str.end(),
                      rhs.m_buttons_str.begin());
  }
}

std::ostream& operator<<(std::ostream& os, const ButtonCombination& buttons)
{
  buttons.print(os);
  return os;
}

/* EOF */
