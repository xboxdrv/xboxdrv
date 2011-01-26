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

#include <boost/tokenizer.hpp>
#include <iostream>

WordWrap::WordWrap(int terminal_width) :
  m_terminal_width(terminal_width)
{
}

void
WordWrap::println(const std::string& str)
{
  std::cout << str << std::endl;
}

void
WordWrap::newline()
{
  std::cout << std::endl;
}

void
WordWrap::para(const std::string& str) const
{
  para("", str);
}

void
WordWrap::para(const std::string& prefix, const std::string& str) const
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(" ", "", boost::drop_empty_tokens));

  int len = prefix.size();
  std::cout << prefix; 
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
  {
    if (len + static_cast<int>(i->size()) + 1 >= m_terminal_width)
    {
      std::cout << std::endl;
      std::cout << prefix;
      len = prefix.size();
    }

    std::cout << *i << " ";
    len += i->size()+1;
  }
  std::cout << std::endl;
}

/* EOF */
