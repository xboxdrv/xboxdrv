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

#include "button_combination.hpp"

#include <assert.h>
#include <algorithm>

#include <strut/split.hpp>

#include "util/string.hpp"
#include "controller_message_descriptor.hpp"

namespace xboxdrv {

ButtonCombination
ButtonCombination::from_string(std::string const& str)
{
  if (str.empty() || str == "void")
  {
    return ButtonCombination();
  }
  else
  {
    return ButtonCombination(strut::split(str, '+'));
  }
}

ButtonCombination::ButtonCombination() :
  m_buttons_str(),
  m_buttons()
{
}

ButtonCombination::ButtonCombination(std::string const& button) :
  m_buttons_str(1, button),
  m_buttons()
{
}

ButtonCombination::ButtonCombination(std::vector<std::string> const& buttons) :
  m_buttons_str(buttons),
  m_buttons()
{
}

void
ButtonCombination::init(ControllerMessageDescriptor const& desc)
{
  m_buttons.clear();
  for(ButtonsStr::const_iterator it = m_buttons_str.begin(); it != m_buttons_str.end(); ++it)
  {
    m_buttons.push_back(desc.key().get(*it));
  }
  std::sort(m_buttons.begin(), m_buttons.end());
}

bool
ButtonCombination::has_button(int button) const
{
  return std::find(m_buttons.begin(), m_buttons.end(), button) != m_buttons.end();
}

bool
ButtonCombination::is_subset_of(ButtonCombination const& rhs) const
{
  // check if init() was called
  assert(m_buttons_str.size() == m_buttons.size());
  assert(rhs.m_buttons_str.size() == rhs.m_buttons.size());

  for(Buttons::const_iterator i = m_buttons.begin(); i != m_buttons.end(); ++i)
  {
    if (!rhs.has_button(*i))
    {
      return false;
    }
  }
  return true;
}

int
ButtonCombination::size() const
{
  return static_cast<int>(m_buttons_str.size());
}

bool
ButtonCombination::match(std::bitset<256> const& button_state) const
{
  // check if init() was called
  assert(m_buttons_str.size() == m_buttons.size());

  for(Buttons::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
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
  for(ButtonsStr::const_iterator btn = m_buttons_str.begin(); btn != m_buttons_str.end(); ++btn)
  {
    os << *btn;
    if (btn != m_buttons_str.end()-1)
      os << "+";
  }

  if (false)
  {
    os << " ";
    for(Buttons::const_iterator btn = m_buttons.begin(); btn != m_buttons.end(); ++btn)
    {
      os << *btn;
      if (btn != m_buttons.end()-1)
        os << "+";
    }
  }
}

bool
ButtonCombination::empty() const
{
  return m_buttons_str.empty();
}

bool
ButtonCombination::operator==(ButtonCombination const& rhs) const
{
  // check if init() was called
  assert(m_buttons_str.size() == m_buttons.size());
  assert(rhs.m_buttons_str.size() == rhs.m_buttons.size());

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

std::ostream& operator<<(std::ostream& os, ButtonCombination const& buttons)
{
  buttons.print(os);
  return os;
}

} // namespace xboxdrv

/* EOF */
