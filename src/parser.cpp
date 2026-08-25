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

#include "argpp.hpp"

#include <array>
#include <assert.h>
#include <stdio.h>
#include <ostream>
#include <stdexcept>

#ifndef _WIN32
#  include <sys/ioctl.h>
#endif

#include "prettyprinter.hpp"

namespace {

constexpr int max_column_width = 5;
constexpr int default_terminal_width = 80;

int get_terminal_width()
{
#ifdef _WIN32
  return 80;
#else
  struct winsize w;
  if (ioctl(0, TIOCGWINSZ, &w) < 0)
  {
    return default_terminal_width;
  }
  else
  {
    return w.ws_col;
  }
#endif
}

} // namespace

namespace argpp {

Parser::Parser() :
  m_program(),
  m_usage(),
  m_groups()
{
}

Parser::ParsedOptions
Parser::parse_args(int argc, char** argv) const
{
  ParsedOptions parsed_options;

  for(int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (argv[i][1] == '-')
      {
        // We got a long option
        if (argv[i][2] == '\0')
        {
          // Got a '--', so we stop evaluating arguments
          ++i;
          while(i < argc)
          {
            parsed_options.push_back(ParsedOption{ArgumentType::REST, "", argv[i]});
            ++i;
          }
        }
        else
        {
          std::string opt = argv[i] + 2;
          std::string long_opt;
          std::string long_opt_arg;

          std::string::size_type pos = opt.find('=');

          if (pos != std::string::npos)
          {
            long_opt = opt.substr(0, pos);
            long_opt_arg = opt.substr(pos+1);
          }
          else
          {
            long_opt = opt;
          }

          // Long Option
          Option const* option = lookup_long_option(long_opt);
          if (!option) {
            throw std::runtime_error("unrecognized option '" + std::string(argv[i]) + "'");
          }

          if (option->argument.empty())
          {
            parsed_options.push_back(ParsedOption{option->key, long_opt, ""});
          }
          else
          {
            if (pos != std::string::npos)
            {
              parsed_options.push_back(ParsedOption{option->key, long_opt, long_opt_arg});
            }
            else
            {
              if (i == argc - 1) {
                throw std::runtime_error("option '" + std::string(argv[i]) + "' requires an argument");
              }

              parsed_options.push_back(ParsedOption{option->key, long_opt, argv[i + 1]});
              ++i;
            }
          }
        }
      }
      else
      {
        // We got a short option
        char* p = argv[i] + 1;

        if (*p != '\0')
        {
          // Handle option chains
          while (*p)
          {
            // Short option(s)
            Option const* option = lookup_short_option(*p);
            if (!option) {
              throw std::runtime_error("invalid option -- " + std::string(1, *p));
            }

            if (option->argument.empty())
            {
              parsed_options.push_back(ParsedOption{option->key, std::string(1, *p), ""});
            }
            else
            {
              if (i == argc - 1 || *(p+1) != '\0') {
                throw std::runtime_error("option requires an argument -- " + std::string(1, *p));
              }

              parsed_options.push_back(ParsedOption{option->key, std::string(1, *p), argv[i + 1]});
              ++i;
            }

            ++p;
          }
        }
        else
        {
          parsed_options.push_back(ParsedOption{ArgumentType::REST, "", "-"});
        }
      }
    }
    else
    {
      parsed_options.push_back(ParsedOption{ArgumentType::REST, "", argv[i]});
    }
  }

  return parsed_options;
}

Option const*
Parser::lookup_short_option(char short_option) const
{
  for(auto const& group : m_groups)
  {
    for(auto const& opt : group.m_options)
    {
      if (opt.short_option == short_option) {
        return &opt;
      }
    }
  }
  return nullptr;
}

Option const*
Parser::lookup_long_option(std::string_view long_option) const
{
  for(auto const& group : m_groups)
  {
    for(auto const& opt : group.m_options)
    {
      if (opt.long_option == long_option) {
        return &opt;
      }
    }
  }
  return nullptr;
}

void
Parser::print_help(std::ostream& out) const
{
  const int terminal_width = get_terminal_width();
  const int column_min_width = 8;
  int column_width = column_min_width;

  { // Calculate left column width
    for(auto const& group : m_groups)
    {
      for(auto const& opt : group.m_options)
      {
        int width = 2; // add two leading space
        if (opt.short_option) {
          width += 2; // "-a"
        }

        if (!opt.long_option.empty()) {
          width += static_cast<int>(opt.long_option.size()) + 2; // "--foobar"
        }

        if (!opt.argument.empty()) {
          width += static_cast<int>(opt.argument.size()) + 1;
        }

        column_width = std::max(column_width, width);
      }
    }

    column_width = column_width+2; // add two trailing space
  }

  if (terminal_width < column_width * 3)
  {
    column_width -= (column_width*3 - terminal_width);
    column_width = std::max(column_width, max_column_width);
  }

  PrettyPrinter pprint(terminal_width); // -1 so we have a whitespace on the right side

  out << "Usage: " << m_program << " " <<  m_usage << '\n';

  if (!m_groups.empty()) {
    out << '\n';
  }

  for(auto const& group : m_groups)
  {
    for(auto const& opt : group.m_options)
    {
      if (opt.visible)
      {
        if (opt.key == ArgumentType::TEXT)
        {
          pprint.print(opt.help);
        }
        else if (opt.key == ArgumentType::PSEUDO)
        {
          pprint.print(std::string(column_width, ' '), opt.long_option, opt.help);
        }
        else
        {
          constexpr size_t buffer_size = 256;
          std::array<char, buffer_size> option   = { 0 };
          std::array<char, buffer_size> argument = { 0 };

          if (opt.short_option)
          {
            if (opt.long_option.empty()) {
              snprintf(option.data(), option.size(), "-%c", opt.short_option);
            } else {
              snprintf(option.data(), option.size(), "-%c, --%s", opt.short_option, opt.long_option.c_str());
            }
          }
          else
          {
            snprintf(option.data(), option.size(), "--%s", opt.long_option.c_str());
          }

          if (!opt.argument.empty())
          {
            snprintf(argument.data(), argument.size(), " %s", opt.argument.c_str());
          }

          std::string left_column("  ");
          left_column += option.data();
          left_column += argument.data();
          left_column += " ";

          pprint.print(std::string(column_width, ' '), left_column, opt.help);
        }
      }
    }

    if (&group != &m_groups.back())
    {
      std::cout << '\n';
    }
  }
}

OptionGroup&
Parser::add_usage(std::string_view program, std::string_view usage)
{
  m_program = program;
  m_usage = usage;

  m_groups.emplace_back();
  return m_groups.back();
}

OptionGroup::OptionGroup() :
  m_options()
{
}

OptionGroup&
OptionGroup::add_pseudo(std::string_view left, std::string_view doc)
{
  Option option;

  option.key          = ArgumentType::PSEUDO;
  option.long_option  = left;
  option.help         = doc;
  option.visible      = true;

  m_options.push_back(option);

  return *this;
}

OptionGroup&
OptionGroup::add_newline()
{
  add_text("");

  return *this;
}

OptionGroup&
Parser::add_group(std::string_view text)
{
  m_groups.emplace_back();
  if (!text.empty()) {
    m_groups.back().add_text(text);
  }
  return m_groups.back();
}

OptionGroup&
OptionGroup::add_text(std::string_view text)
{
  Option option;

  option.key          = ArgumentType::TEXT;
  option.help         = text;
  option.visible      = true;

  m_options.push_back(option);

  return *this;
}

OptionGroup&
OptionGroup::add_option(char short_option,
                      std::string_view long_option,
                      std::string_view argument,
                      std::string_view help,
                      bool visible)
{
  return add_option(short_option, short_option, long_option, argument, help, visible);
}

OptionGroup&
OptionGroup::add_option(int key,
                      char short_option,
                      std::string_view long_option,
                      std::string_view argument,
                      std::string_view help,
                      bool visible)
{
  assert(short_option || (!short_option && !long_option.empty()));

  Option option;

  option.key          = key;
  option.short_option = short_option;
  option.long_option  = long_option;
  option.help         = help;
  option.argument     = argument;
  option.visible      = visible;

  m_options.push_back(option);

  return *this;
}

} // namespace argpp

/* EOF */
