/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <iostream>

#include "helper.hpp"
#include "pretty_printer.hpp"

PrettyPrinter::PrettyPrinter(int width_)
  : width(width_)
{
}

void
PrettyPrinter::print(const std::string& str)
{
  print("", "", str);
}

void 
PrettyPrinter::print(const std::string& indent_str, const std::string& left, const std::string& str)
{
  if (!left.empty())
    {
      if (left.size() < indent_str.size())
        {
          std::cout << left << std::string(indent_str.size() - left.size(), ' ');
        }
      else
        {
          std::cout << left << '\n' << indent_str;
        }
    }
  else
    {
      std::cout << indent_str;
    }

  // skip leading space
  std::string::size_type start = str.find_first_not_of(' ', 0);
  
  std::string::size_type word_begin = 0;
  int word_begin_column = 0;
  enum { SPACE, WORD } state = isspace(str[0]) ? SPACE : WORD;

  for(std::string::size_type i = start; i < str.size(); ++i)
    {
      const int word_length = i - word_begin;

      { // flush a word or a space sequence to stdout when a state change occurs
        switch(state)
          {
            case SPACE:            
              if (!isspace(str[i]))
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
                      //std::cout << "(" << i - word_begin << "," << word_begin_column << ")";

                      std::cout << str.substr(word_begin, i - word_begin);

                      word_begin = i;
                      word_begin_column += word_length;
                    }
                }
              break;

            case WORD:
              if (isspace(str[i]))
                { // flush
                  state = SPACE;

                  //std::cout << "(" << i - word_begin << "," << word_begin_column << ")";

                  std::cout << str.substr(word_begin, i - word_begin);
                  word_begin = i;
                  word_begin_column += word_length;
                }
              break;
          }
      }

      { // process the current character           
        if (str[i] == '\n')
          {
            std::cout << '\n' << indent_str;
            word_begin = i+1;
            word_begin_column = 0;
          }
        else if (word_begin_column + word_length >= width)
          {
            std::cout << '\n' << indent_str;
            word_begin_column = 0;
          }
      }
    }

  std::cout << str.substr(word_begin);
  std::cout << std::endl;
}


#ifdef __TEST__

int main(int argc, char** argv)
{
  PrettyPrinter printer(16, get_terminal_width() - 16);

  printer.print(" -h, --help",
                "This program is free software: you can redistribute it and/or modify "
                "it under the terms of the GNU General Public License as published by "
                "the Free Software Foundation, either version 3 of the License, or "
                "(at your option) any later version.\n"
                "\n"
                "\n"
                "You should have received a copy of the GNU General Public License "
                "along with this program.");
  return 0;
}

#endif

/* EOF */
