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

#ifndef HEADER_HELPER_HPP
#define HEADER_HELPER_HPP

#include <iosfwd>
#include <vector>
#include <boost/function.hpp>

void print_raw_data(std::ostream& out, uint8_t* buffer, int len);
std::string to_lower(const std::string &str);
bool is_number(const std::string& str);
void arg2apply(const std::string& str, const boost::function<void (const std::string&)>& func);

/** Convert the given string \a str to an integer, the string can
    either be an exact integer or a percent value (i.e. "75%"), in
    which case it is handled as (range * int(str)) */
int to_number(int range, const std::string& str);
uint32_t get_time();

template<class C, class Func>
void arg2vector(const std::string& str, typename std::vector<C>& lst, Func func)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == ',')
        {
          if (i != start)
            lst.push_back(func(std::string(start, i)));
          
          start = i+1;
        }
    }
  
  if (start != str.end())
    lst.push_back(func(std::string(start, str.end())));
}

namespace Math {
template<class T>
T clamp (const T& low, const T& v, const T& high)
{
  assert(low <= high);
  return std::max((low), std::min((v), (high)));
}
} // namespace Math

#endif

/* EOF */
