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

#ifndef HEADER_XBOXDRV_WORD_WRAP_HPP
#define HEADER_XBOXDRV_WORD_WRAP_HPP

#include <iostream>

class WordWrap
{
private:
  const int m_terminal_width;
  const std::string m_prefix;
  const std::string m_separator;

  int m_column;
  bool m_first_item;

public:
  WordWrap(int terminal_width, const std::string& prefix, const std::string& separator);

  void add_item(const std::string& str);

private:
  WordWrap(const WordWrap&);
  WordWrap& operator=(const WordWrap&);
};

#endif

/* EOF */
