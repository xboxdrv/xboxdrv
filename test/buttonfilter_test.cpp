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

#include <iostream>

#include "button_filter.hpp"
#include "util/string.hpp"

int main(int argc, char** argv)
{
  if (argc < 2 || argc > 3)
  {
    std::cout << argv[0] << " FILTER[^FILTER]... [DURATION]" << std::endl;
    return 0;
  }
  else
  {
    std::vector<ButtonFilterPtr> filters;

    std::string str = argv[1];
    int duration = (argc == 3) ? std::stoi(argv[2]) : 0;

    auto tokens = string_split(str, "^");
    for(auto t = tokens.begin(); t != tokens.end(); ++t)
    {
      filters.push_back(ButtonFilter::from_string(*t));
    }

    std::vector<int> values;

    std::string line;
    while(std::getline(std::cin, line))
    {
      values.push_back(std::stoi(line));
    }

    int time_step = duration / values.size();

    for(std::vector<int>::const_iterator i = values.begin(); i != values.end(); ++i)
    {
      int value = *i;

      // filter the value
      for(std::vector<ButtonFilterPtr>::iterator filter = filters.begin(); filter != filters.end(); ++filter)
      {
        value = (*filter)->filter(value);
      }

      // advance the internal clock
      for(std::vector<ButtonFilterPtr>::iterator filter = filters.begin(); filter != filters.end(); ++filter)
      {
        (*filter)->update(time_step);
      }

      std::cout << value << std::endl;
    }

    return 0;
  }
}

/* EOF */
