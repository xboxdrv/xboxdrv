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

#include "word_wrap.hpp"

WordWrap::WordWrap(int terminal_width, 
                   const std::string& prefix, 
                   const std::string& separator) :
  m_terminal_width(terminal_width),
  m_prefix(prefix),
  m_separator(separator),
  m_column(0),
  m_first_item(true)
{
}

void
WordWrap::add_item(const std::string& str)
{
  // FIXME: very incomplete, should do proper word wrap, joining
  // should probably be moved to a different class.

  // print separator
  if (!m_first_item)
  {
    std::cout << m_separator;
  }
  else
  {
    m_first_item = false;
  }

    std::cout << str;

  /*
  if (m_column == 0)
  {
    std::cout << m_prefix;
    m_column += m_prefix.size();
  }

  if (m_column + static_cast<int>(str.length()) > m_terminal_width)
  {
    std::cout << std::endl;
    m_column = 0;
    add_item(str);
  }
  else
  {
    std::cout << str;
    m_column += str.size();
    }*/
}

/* EOF */
