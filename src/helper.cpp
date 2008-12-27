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
      *i = tolower(*i);
    }

  return lower_impl;
}

/* EOF */
