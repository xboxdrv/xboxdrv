/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include "prettyprinter.hpp"

#include <iostream>

namespace argpp {

PrettyPrinter::PrettyPrinter(int terminal_width, std::ostream& out) :
  m_terminal_width(terminal_width),
  m_out(out)
{
}

void
PrettyPrinter::print(std::string_view text) const
{
  print("", "", text);
}

void
PrettyPrinter::print(std::string_view indent, std::string_view initial, std::string_view text) const
{
  const int width = m_terminal_width - static_cast<int>(indent.size()) - 1;

  if (!initial.empty())
  {
    if (initial.size() < indent.size())
    {
      m_out << initial << std::string(indent.size() - initial.size(), ' ');
    }
    else
    {
      m_out << initial << '\n' << indent;
    }
  }
  else
  {
    m_out << indent;
  }

  // skip leading space
  std::string::size_type start = text.find_first_not_of(' ', 0);

  std::string::size_type word_begin = 0;
  int word_begin_column = 0;
  enum { SPACE, WORD } state = isspace(text[0]) ? SPACE : WORD;

  for(std::string::size_type i = start; i < text.size(); ++i)
  {
    const int word_length = static_cast<int>(i - word_begin);

    { // flush a word or a space sequence to stdout when a state change occurs
      switch(state)
      {
        case SPACE:
          if (!isspace(text[i]))
          { // flush
            state = WORD;

            if (word_begin_column == 0)
            {
              // ignore space at the start of a new line

              word_begin = i;
              word_begin_column = 0;
            }
            else
            {
              //m_out << "(" << i - word_begin << "," << word_begin_column << ")";

              m_out << text.substr(word_begin, i - word_begin);

              word_begin = i;
              word_begin_column += word_length;
            }
          }
          break;

        case WORD:
          if (isspace(text[i]))
          { // flush
            state = SPACE;

            //m_out << "(" << i - word_begin << "," << word_begin_column << ")";

            m_out << text.substr(word_begin, i - word_begin);
            word_begin = i;
            word_begin_column += word_length;
          }
          break;
      }
    }

    { // process the current character
      if (text[i] == '\n')
      {
        m_out << '\n' << indent;
        word_begin = i+1;
        word_begin_column = 0;
      }
      else if (word_begin_column + word_length >= width)
      {
        m_out << '\n' << indent;
        word_begin_column = 0;
      }
    }
  }

  m_out << text.substr(word_begin);
  m_out << std::endl;
}

} // namespace argpp

/* EOF */
