// yaini - Yet another INI parser
// Copyright (C) 2010-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_YAINI_PARSER_HPP
#define HEADER_YAINI_PARSER_HPP

#include <sstream>

namespace yaini {

class Builder;

class Parser
{
private:
  std::istream& m_in;
  Builder& m_builder;
  std::string m_context;
  int m_line;
  int m_column;
  int m_current_char;

public:
  Parser(std::istream& in, Builder& builder, const std::string& context);

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
  std::string get_value_or_string();
  std::string get_ident_or_string();
  void newline();
  void eat_rest_of_line();
  std::string get_section();
  void whitespace();
  int  getchar();

private:
  Parser(const Parser&);
  Parser& operator=(const Parser&);
};

} // namespace yaini

#endif

/* EOF */
