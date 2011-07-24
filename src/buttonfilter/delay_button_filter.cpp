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

#include "buttonfilter/delay_button_filter.hpp"

#include <boost/lexical_cast.hpp>

DelayButtonFilter*
DelayButtonFilter::from_string(const std::string& str)
{
  return new DelayButtonFilter(boost::lexical_cast<int>(str));
}

DelayButtonFilter::DelayButtonFilter(int delay) :
  m_delay(delay),
  m_time(0)
{
}

bool
DelayButtonFilter::filter(bool value)
{
  if (value)
  {
    if (m_time < m_delay)
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    m_time = 0;
    return false;
  }
}

void
DelayButtonFilter::update(int msec_delta)
{
  m_time += msec_delta;
}

std::string
DelayButtonFilter::str() const
{
  std::ostringstream os;
  os << "delay:" << m_delay;
  return os.str();
}

/* EOF */
