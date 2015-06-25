/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include <boost/bind.hpp>
#include <iostream>

#include "helper.hpp"

void print_name_value(const std::string& name, const std::string& value)
{
  std::cout << "'" << name << "' = '" << value << "'" << std::endl;
}

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
  {
    process_name_value_string(argv[i], boost::bind(&print_name_value, _1, _2));
  }
  return 0;
}

/* EOF */
