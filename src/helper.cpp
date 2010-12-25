/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
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
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include "helper.hpp"

void print_raw_data(std::ostream& out, uint8_t* data, int len)
{
  std::cout << "len: " << len 
            << " data: ";
      
  for(int i = 0; i < len; ++i)
    std::cout << boost::format("0x%02x ") % int(data[i]);

  std::cout << std::endl;
}

std::string to_lower(const std::string &str)
{
  std::string lower_impl = str;

  for( std::string::iterator i = lower_impl.begin();
       i != lower_impl.end();
       ++i )
  {
    *i = static_cast<char>(tolower(*i));
  }

  return lower_impl;
}

void split_string_at(const std::string& str, char c, std::string* lhs, std::string* rhs)
{
  std::string::size_type p = str.find(c);
  if (p == std::string::npos)
  {
    *lhs = str;
  }
  else
  {
    *lhs = str.substr(0, p);
    *rhs = str.substr(p+1);
  }
}

void process_name_value_string(const std::string& str, const boost::function<void (const std::string&, const std::string&)>& func)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(",", "", boost::keep_empty_tokens));

  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
  {
    std::string lhs, rhs;
    split_string_at(*i, '=', &lhs, &rhs);
    func(lhs, rhs);
  }
}

void arg2apply(const std::string& str, const boost::function<void (const std::string&)>& func)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
  {
    if (*i == ',')
    {
      if (i != start)
        func(std::string(start, i));
          
      start = i+1;
    }
  }
  
  if (start != str.end())
    func(std::string(start, str.end()));
}

bool is_number(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    if (!isdigit(*i))
      return false;
  return true;
}

int to_number(int range, const std::string& str)
{
  if (str.empty())
  {
    return 0;
  }
  else
  {
    if (str[str.size() - 1] == '%')
    {
      int percent = boost::lexical_cast<int>(str.substr(0, str.size()-1));
      return range * percent / 100;
    }
    else
    {
      return boost::lexical_cast<int>(str);
    }
  }
}

uint32_t get_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

int get_terminal_width()
{
  struct winsize w;
  if (ioctl(0, TIOCGWINSZ, &w) < 0)
  {
    return 80;
  }
  else
  {
    return w.ws_col;
  }
}

/* EOF */
