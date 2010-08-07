/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_INI_PARSER_HPP
#define HEADER_XBOXDRV_INI_PARSER_HPP

#include <map>
#include <string>
#include <sstream>

class INIBuilder;

class INIParser
{
private:
  std::istream& m_in;
  INIBuilder& m_builder;
  std::string m_context;
  int m_line;
  int m_column;
  int m_current_char;

public:
  INIParser(std::istream& in, INIBuilder& builder, const std::string& context);
  
  void run();

private:
  void error(const std::string& message);
  int  peek();
  void next();
  bool accept(char c);
  void expect(char c);
  std::string get_string();
  std::string get_value();
  std::string get_ident(); 
  void newline();
  void eat_rest_of_line();
  std::string get_section();
  void whitespace();
  int  getchar();  

private:
  INIParser(const INIParser&);
  INIParser& operator=(const INIParser&);
};

#endif

/* EOF */
