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

#include "word_wrap.hpp"

#include <iostream>

#include <strut/tokenize.hpp>

#include "util/string.hpp"

namespace xboxdrv {

WordWrap::WordWrap(int terminal_width) :
  m_terminal_width(terminal_width)
{
}

void
WordWrap::println(std::string const& str)
{
  std::cout << str << std::endl;
}

void
WordWrap::newline()
{
  std::cout << std::endl;
}

void
WordWrap::para(std::string const& str) const
{
  para("", str);
}

void
WordWrap::para(std::string const& prefix, std::string const& str) const
{
  auto tokens = strut::tokenize(str, ' ');

  int len = static_cast<int>(prefix.size());
  std::cout << prefix;
  for(auto i = tokens.begin(); i != tokens.end(); ++i)
  {
    if (len + static_cast<int>(i->size()) + 1 >= m_terminal_width)
    {
      std::cout << std::endl;
      std::cout << prefix;
      len = static_cast<int>(prefix.size());
    }

    std::cout << *i << " ";
    len += static_cast<int>(i->size()) + 1;
  }
  std::cout << std::endl;
}

} // namespace xboxdrv

/* EOF */
