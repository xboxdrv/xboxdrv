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

#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

ButtonFilterPtr
ButtonFilter::from_string(const std::string& str)
{
  std::string::size_type p = str.find(":");
  std::string filtername = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (filtername == "toggle")
  {
    return ButtonFilterPtr(new ToggleButtonFilter);
  }
  else if (filtername == "invert")
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

ToggleButtonFilter::ToggleButtonFilter() :
  m_state(false),
  m_last_value(false)
{
}

bool
ToggleButtonFilter::filter(bool value)
{
  if (value != m_last_value)
  {
    if (value)
    {
      m_state = !m_state;
    }

    m_last_value = value;
  }
  return m_state;
}   

bool
InvertButtonFilter::filter(bool value)
{
  return !value;
}

AutofireButtonFilter*
AutofireButtonFilter::from_string(const std::string& str)
{
  int frequency = 50;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0: frequency = boost::lexical_cast<int>(*t); break;
      default: throw std::runtime_error("to many arguments"); break;
    }
  }

  return new AutofireButtonFilter(frequency);
}

AutofireButtonFilter::AutofireButtonFilter(int frequency) :
  m_state(false),
  m_frequency(frequency),
  m_counter(0)
{
}

void
AutofireButtonFilter::update(int msec_delta)
{
  if (m_state)
  {
    m_counter += msec_delta;
  }
}

bool
AutofireButtonFilter::filter(bool value)
{
  m_state = value;

  if (!value)
  {
    m_counter = 0;
    return false;
  }
  else
  {
    // FIXME: should fire event at 0 not m_frequency

    // auto fire
    if (m_counter > m_frequency)
    {
      m_counter = 0;
      return true;
    }
    else
    {
      return false;
    }
  }
}

LogButtonFilter*
LogButtonFilter::from_string(const std::string& str)
{
  return new LogButtonFilter(str);
}

LogButtonFilter::LogButtonFilter(const std::string& name) :
  m_name(name)
{
}

bool
LogButtonFilter::filter(bool value)
{
  if (m_name.empty())
  {
    std::cout << value << std::endl;
  }
  else
  {
    std::cout << m_name << ": " << value << std::endl;
  }

  return value;
}

/* EOF */
