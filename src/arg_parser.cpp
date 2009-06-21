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

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <boost/format.hpp>

#include "arg_parser.hpp"

ArgParser::ArgParser()
{
}

ArgParser::ParsedOptions
ArgParser::parse_args(int argc, char** argv)
{
  ParsedOptions parsed_options;

  programm = argv[0];

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
                      parsed_options.push_back(ParsedOption(ArgParser::REST_ARG, "", argv[i]));
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
                  Option* option = lookup_long_option(long_opt);

                  if (option) 
                    {
                      if (option->argument.empty()) 
                        {
                          parsed_options.push_back(ParsedOption(option->key, long_opt, ""));
                        } 
                      else
                        {
                          if (pos != std::string::npos) 
                            {
                              parsed_options.push_back(ParsedOption(option->key, long_opt, long_opt_arg));
                            }
                          else
                            {            
                              if (i == argc - 1) 
                                {
                                  throw std::runtime_error("option '" + std::string(argv[i]) + "' requires an argument");
                                }
                              else 
                                {
                                  parsed_options.push_back(ParsedOption(option->key, long_opt, argv[i + 1]));
                                  ++i;
                                }
                            }
                        }
                    }
                  else
                    {
                      throw std::runtime_error("unrecognized option '" + std::string(argv[i]) + "'");
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
                      Option* option = lookup_short_option(*p);

                      if (option) 
                        {
                          if (option->argument.empty()) 
                            {
                              parsed_options.push_back(ParsedOption(option->key, std::string(1, *p), ""));
                            } 
                          else 
                            {
                              if (i == argc - 1 || *(p+1) != '\0') 
                                {
                                  // No more arguments
                                  throw std::runtime_error("option requires an argument -- " + std::string(1, *p));
                                }
                              else
                                {
                                  parsed_options.push_back(ParsedOption(option->key, std::string(1, *p), argv[i + 1]));
                                  ++i;
                                }
                            }
                        } 
                      else 
                        {
                          throw std::runtime_error("invalid option -- " + std::string(1, *p));
                        }
                      ++p; 
                    }
                } 
              else
                {
                  parsed_options.push_back(ParsedOption(ArgParser::REST_ARG, "", "-"));
                }
            }
        } 
      else
        {
          parsed_options.push_back(ParsedOption(ArgParser::REST_ARG, "", argv[i]));
        }
    }

  return parsed_options;
}

ArgParser::Option*
ArgParser::lookup_short_option(char short_option)
{
  for(Options::iterator i = options.begin(); i != options.end(); ++i)
    {
      if (i->short_option == short_option)
        return &(*i);
    }
  return 0;
}

ArgParser::Option*
ArgParser::lookup_long_option(const std::string& long_option)
{
  for(Options::iterator i = options.begin(); i != options.end(); ++i)
    {
      if (i->long_option == long_option)
        return &*i;
    }
  return 0;
}

void
ArgParser::print_help(std::ostream& out) const
{
  bool first_usage = true;
  for(Options::const_iterator i = options.begin(); i != options.end(); ++i)
    {
      if (i->visible)
        {
          if (i->key == USAGE) 
            {
              if (first_usage) 
                {
                  out << "Usage: " << programm << " " <<  i->help << std::endl; 
                  first_usage = false;
                }
              else
                {
                  out << "       " << programm << " " << i->help << std::endl; 
                }
            } 
          else if (i->key == TEXT) 
            {
              out << i->help << std::endl;
            }
          else 
            {
              char option[256]   = { 0 };
              char argument[256] = { 0 };

              if (i->short_option)
                {
                  if (i->long_option.empty())
                    snprintf(option, 256, "-%c", i->short_option);
                  else
                    snprintf(option, 256, "-%c, --%s", i->short_option, i->long_option.c_str());
                }
              else
                {
                  snprintf(option, 256, "--%s", i->long_option.c_str());
                }

              if (!i->argument.empty())
                {
                  if (i->long_option.empty())
                    snprintf(argument, 256, " %s", i->argument.c_str());
                  else
                    snprintf(argument, 256, " %s", i->argument.c_str());
                }

              out << "  " 
                  << std::setiosflags(std::ios::left) << std::setw(24) // FIXME: Calculate this dynamically
                  << (std::string(option) + std::string(argument)) << std::setw(0)
                  << " " << i->help << std::endl;
            }
        }
    }
}

ArgParser&
ArgParser::add_usage(const std::string& usage)
{
  Option option;

  option.key          = USAGE;
  option.help         = usage;
  option.visible      = true;

  options.push_back(option);

  return *this;
}

ArgParser&
ArgParser::add_newline()
{
  add_text("");
  
  return *this;
}

ArgParser&
ArgParser::add_text(const std::string& grouptopic)
{
  Option option;

  option.key          = TEXT;
  option.help         = grouptopic;
  option.visible      = true;

  options.push_back(option);  

  return *this;
}

ArgParser&
ArgParser::add_option(int key, 
                      char short_option, 
                      const std::string& long_option, 
                      const std::string& argument,
                      const std::string& help,
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

  options.push_back(option);

  return *this;
}

#ifdef __TEST__

// g++ src/arg_parser.cpp -o arg_parser_test -Wall -O2 -Werror -D__TEST__

int main(int argc, char** argv)
{
  try
    {
      ArgParser argp;
  
      argp
        .add_usage("bar [FILES]... [BLA]..")
        .add_usage("foo [FILES]... [BLA]..")
        .add_text("Dies und das")
        .add_newline()
        .add_option(1, 'v', "version", "", "Help text");

      argp.parse_args(argc, argv);

      argp.print_help(std::cout);

      return 0;
    }
  catch(std::exception& err)
    {
      std::cout << "Error: " << err.what() << std::endl;
    }
}

#endif

/* EOF */
