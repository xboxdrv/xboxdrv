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

#include "helper.hpp"

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdexcept>

#include "raise_exception.hpp"

int hexstr2int(const std::string& str)
{
  unsigned int value = 0;
  if (sscanf(str.c_str(), "%x", &value) == 1)
  {
    return value;
  }
  else if (sscanf(str.c_str(), "0x%x", &value) == 1)
  {
    return value;
  }
  else
  {
    raise_exception(std::runtime_error, "couldn't convert '" << str << "' to int");
  }
}

std::string raw2str(uint8_t* data, int len)
{
  std::ostringstream out;
  out << "len: " << len 
      << " data: ";
  
  for(int i = 0; i < len; ++i)
    out << boost::format("%02x ") % int(data[i]);

  return out.str();
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
  tokenizer tokens(str, boost::char_separator<char>(",", "", boost::drop_empty_tokens));

  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i)
  {
    std::string lhs, rhs;
    split_string_at(*i, '=', &lhs, &rhs);
    func(lhs, rhs);
  }
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

float to_float(int value, int min, int max)
{
  assert(value >= min);
  assert(value <= max);

  // FIXME: '+1' is kind of a hack to
  // get the center at 0 for the
  // [-32768, 32767] case
  int center = (max + min + 1)/2;

  if (value < center)
  {
    return static_cast<float>(value - center) / static_cast<float>(center - min);
  }
  else // (value >= center)
  {
    return static_cast<float>(value - center) / static_cast<float>(max - center);
  }
}

int from_float(float value, int min, int max)
{
  return (value + 1.0f) / 2.0f * static_cast<float>(max - min) + min;
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
