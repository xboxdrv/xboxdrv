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

#ifndef HEADER_ARG_PARSER_HPP
#define HEADER_ARG_PARSER_HPP

#include <string>
#include <iostream>
#include <vector>

class ArgParser
{
private:
  struct Option
  {
    int key;
    char        short_option;
    std::string long_option;
    std::string help;
    std::string argument;
    bool visible;

    Option() :
      key(),
      short_option(),
      long_option(),
      help(),
      argument(),
      visible()
    {}
  };

public:
  struct ParsedOption
  {
    int key;
    std::string option;
    std::string argument;

    ParsedOption() : 
      key(-1),
      option(),
      argument()
    {}

    ParsedOption(int key_, const std::string& option_, const std::string& argument_) :
      key(key_), 
      option(option_),
      argument(argument_)
    {}
  };

  enum {
    REST_ARG  = -1,
    TEXT      = -4,
    USAGE     = -5
  };

private:
  std::string programm;
  
  typedef std::vector<Option> Options;
  Options options;

public:  
  typedef std::vector<ParsedOption> ParsedOptions;

  ArgParser();

  ArgParser& add_usage(const std::string& usage);
  ArgParser& add_text(const std::string& doc);
  ArgParser& add_newline();
  
  ArgParser& add_option(int key,
                        char short_option,
                        const std::string& long_option, 
                        const std::string& argument,
                        const std::string& help,
                        bool visible = true);

  ParsedOptions parse_args(int argc, char** argv);
  void print_help(std::ostream& out = std::cout) const;
  
  bool next();
  int  get_key();
  std::string get_argument();

private:
  void read_option(int id, const std::string& argument);

  /** Find the Option structure that matches \a short_option */
  Option* lookup_short_option(char short_option);

  /** Find the Option structure that matches \a long_option */
  Option* lookup_long_option (const std::string& long_option);
};

#endif

/* EOF */
