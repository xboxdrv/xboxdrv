/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
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

#ifndef HEADER_HELPER_HPP
#define HEADER_HELPER_HPP

#include <boost/function.hpp>
#include <stdint.h>
#include <vector>

int hexstr2int(const std::string& str);
uint16_t hexstr2uint16(const std::string& str);

std::string raw2str(const uint8_t* buffer, int len);
std::string to_lower(const std::string &str);
bool is_number(const std::string& str);
bool is_float(const std::string& str);

/**
   Splits apart a string of the form "NAME=VALUE,..." and calls
   func(NAME, VALUE) on each.

   When VALUE is supposed to contain a "," the value has to be quoted
   with [], i.e. "NAME=[VALUE1,VALUE2]", the "[" and "]" itself can
   be quoted with "\[" and "\]"
*/
void process_name_value_string(const std::string& str, const boost::function<void (const std::string&, const std::string&)>& func);

/** Split \a str at characters c */
std::vector<std::string> split_string_at_comma(const std::string& str);

void split_string_at(const std::string& str, char c, std::string* lhs, std::string* rhs);

/**
   Convert the given string \a str to an integer, the string can
   either be an exact integer or a percent value (i.e. "75%"), in
   which case it is handled as (range * int(str))
*/
int to_number(int range, const std::string& str);
uint32_t get_time();

namespace Math {
template<class T>
T clamp (const T& low, const T& v, const T& high)
{
  assert(low <= high);
  return std::max((low), std::min((v), (high)));
}
} // namespace Math

/** converts the arbitary range to [-1,1] */
float to_float(int value, int min, int max);
float to_float_no_range_check(int value, int min, int max);

/** converts the range [-1,1] to [min,max] */
int from_float(float value, int min, int max);

int get_terminal_width();
pid_t spawn_exe(const std::vector<std::string>& args);
pid_t spawn_exe(const std::string& arg0);

#endif

/* EOF */
