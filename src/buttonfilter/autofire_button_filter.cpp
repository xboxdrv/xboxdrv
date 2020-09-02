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

#include "buttonfilter/autofire_button_filter.hpp"

#include <sstream>

#include "util/string.hpp"

AutofireButtonFilter*
AutofireButtonFilter::from_string(const std::string& str)
{
  int rate  = 50;
  int delay = 0;

  auto tokens = string_split(str, ":");
  int idx = 0;
  for(auto t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0: rate  = str2int(*t); break;
      case 1: delay = str2int(*t); break;
      default: throw std::runtime_error("to many arguments"); break;
    }
  }

  return new AutofireButtonFilter(rate, delay);
}

AutofireButtonFilter::AutofireButtonFilter(int rate, int delay) :
  m_state(false),
  m_autofire(false),
  m_rate(rate),
  m_delay(delay),
  m_counter(0)
{
}

void
AutofireButtonFilter::update(int msec_delta)
{
  if (m_state)
  {
    m_counter += msec_delta;

    if (m_counter > m_delay)
    {
      m_autofire = true;
    }
  }
}

bool
AutofireButtonFilter::filter(bool value)
{
  m_state = value;

  if (!value)
  {
    m_counter  = 0;
    m_autofire = false;
    return false;
  }
  else
  { // auto fire
    if (m_autofire)
    {
      if (m_counter > m_rate)
      {
        m_counter = 0;
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return true;
    }
  }
}

std::string
AutofireButtonFilter::str() const
{
  std::ostringstream out;
  out << "auto:" << m_rate << ":" << m_delay;
  return out.str();
}

/* EOF */
