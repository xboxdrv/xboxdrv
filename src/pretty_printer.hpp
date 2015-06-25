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

#ifndef HEADER_PRETTY_PRINTER_HPP
#define HEADER_PRETTY_PRINTER_HPP

#include <string>

class PrettyPrinter
{
private:
  int terminal_width;

public:
  PrettyPrinter(int terminal_width);

  void print(const std::string& str);
  void print(const std::string& indent_str, const std::string& left, const std::string& str);

private:
  PrettyPrinter(const PrettyPrinter&);
  PrettyPrinter& operator=(const PrettyPrinter&);
};

#endif

/* EOF */
