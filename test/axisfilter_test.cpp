/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "axis_filter.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << argv[0] << " FILTER[^FILTER]..." << std::endl;
    return 0;
  }
  else
  {
    std::vector<AxisFilterPtr> filters;

    std::string str = argv[1];

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tokens(str, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
    for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t)
    {
      filters.push_back(AxisFilter::from_string(*t));
    }

    std::string line;
    while(std::getline(std::cin, line))
    {
      int value = boost::lexical_cast<int>(line);
      for(std::vector<AxisFilterPtr>::iterator i = filters.begin(); i != filters.end(); ++i)
      {
        value = (*i)->filter(value, -32768, 32767);
      }
      std::cout << value << std::endl;
    }

    return 0;
  }
}

/* EOF */
