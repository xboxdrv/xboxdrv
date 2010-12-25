/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "button_filter.hpp"

#include <sstream>
#include <boost/tokenizer.hpp>

ButtonFilterPtr
ButtonFilter::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int k = 0;
  
  std::string filtername;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++k)
  {
    if (k == 0)
    {
      filtername = *t;
    }
    else
    {
      
    }
  }

  if (filtername == "toggle")
  {
    return ButtonFilterPtr(new ToggleButtonFilter);
  }
  else
  {
    std::ostringstream out;
    out << "unknown ButtonFilter '" << filtername << "'";
    throw std::runtime_error(out.str());
  }
}

ToggleButtonFilter::ToggleButtonFilter() :
  m_state(false)
{
}

bool
ToggleButtonFilter::filter(bool value)
{
  if (value)
  {
    m_state = !m_state;
  }
   
  return m_state;
}   

/* EOF */
