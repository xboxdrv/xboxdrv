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

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "buttonfilter/autofire_button_filter.hpp"
#include "buttonfilter/invert_button_filter.hpp"
#include "buttonfilter/log_button_filter.hpp"
#include "buttonfilter/toggle_button_filter.hpp"

ButtonFilterPtr
ButtonFilter::from_string(const std::string& str)
{
  std::string::size_type p = str.find(":");
  std::string filtername = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (filtername == "toggle" || filtername == "tog")
  {
    return ButtonFilterPtr(new ToggleButtonFilter);
  }
  else if (filtername == "invert" || filtername == "inv")
  {
    return ButtonFilterPtr(new InvertButtonFilter);
  }
  else if (filtername == "auto" || filtername == "autofire")
  {
    return ButtonFilterPtr(AutofireButtonFilter::from_string(rest));
  }
  else if (filtername == "log")
  {
    return ButtonFilterPtr(LogButtonFilter::from_string(rest));
  }
  else
  {
    std::ostringstream out;
    out << "unknown ButtonFilter '" << filtername << "'";
    throw std::runtime_error(out.str());
  }
}

/* EOF */
