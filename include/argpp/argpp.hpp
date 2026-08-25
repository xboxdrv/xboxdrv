// ArgParse - A Command Line Argument Parser for C++
// Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_ARGPARSER_ARG_PARSER_HPP
#define HEADER_ARGPARSER_ARG_PARSER_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace argpp {

struct Option
{
  int key = {};
  char short_option = {};
  std::string long_option = {};
  std::string help = {};
  std::string argument = {};
  bool visible = {};
};

struct ParsedOption
{
  int key = -1;
  std::string option = {};
  std::string argument = {};
};

namespace ArgumentType {
enum {
  REST = -1,
  TEXT = -4,
  PSEUDO = -5
};
} // namespace ArgumentType

class OptionGroup
{
  friend class Parser;
public:
  OptionGroup();

  OptionGroup& add_text(std::string_view text);
  OptionGroup& add_pseudo(std::string_view left, std::string_view doc);
  OptionGroup& add_newline();

  OptionGroup& add_option(int key,
                          char short_option,
                          std::string_view long_option,
                          std::string_view argument,
                          std::string_view help,
                          bool visible = true);
  OptionGroup& add_option(char short_option,
                          std::string_view long_option,
                          std::string_view argument,
                          std::string_view help,
                          bool visible = true);

private:
  std::vector<Option> m_options;
};

class Parser
{
  typedef std::vector<ParsedOption> ParsedOptions;

public:
  Parser();

  OptionGroup& add_usage(std::string_view program, std::string_view usage);
  OptionGroup& add_group(std::string_view text = {});

  ParsedOptions parse_args(int argc, char** argv) const;
  void print_help(std::ostream& out = std::cout) const;

private:
  void read_option(int id, std::string_view argument);

  /** Find the Option structure that matches \a short_option */
  Option const* lookup_short_option(char short_option) const;

  /** Find the Option structure that matches \a long_option */
  Option const* lookup_long_option (std::string_view long_option) const;

private:
  std::string m_program;
  std::string m_usage;
  std::vector<OptionGroup> m_groups;
};

} // namespace argpp

#endif

/* EOF */
