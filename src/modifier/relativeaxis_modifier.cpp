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

#include "relativeaxis_modifier.hpp"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

RelativeAxisMapping
RelativeAxisMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  /* Format of str: A={SPEED} */
  RelativeAxisMapping mapping;
  mapping.m_axis  = string2axis(lhs);
  mapping.m_speed = boost::lexical_cast<int>(rhs);
  // FIXME: insert some error checking here
  return mapping;
}

RelativeAxisMapping::RelativeAxisMapping() :
  m_axis(XBOX_AXIS_UNKNOWN),
  m_speed(0),
  m_axis_state(0)
{
}

void
RelativeAxisMapping::update(int msec_delta, XboxGenericMsg& msg)
{
  int value = get_axis(msg, m_axis);
  if (abs(value) > 4000 ) // FIXME: add proper deadzone handling
  {
    m_axis_state += ((m_speed * value) / 32768) * msec_delta / 1000;
    if (m_axis_state < -32768)
      m_axis_state = -32768;
    else if (m_axis_state > 32767)
      m_axis_state = 32767;

    set_axis(msg, m_axis, m_axis_state);
  }
  else
  {
    set_axis(msg, m_axis, m_axis_state);
  }
}

RelativeAxisModifier::RelativeAxisModifier(const std::vector<RelativeAxisMapping>& relative_axis_map) :
  m_relative_axis_map(relative_axis_map),
  m_axis_state()
{
  for(size_t i = 0; i < m_relative_axis_map.size(); ++i)
  {
    m_axis_state.push_back(0);
  }
}

void
RelativeAxisModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  for(size_t i = 0; i < m_relative_axis_map.size(); ++i)
  {
    int value = get_axis(msg, m_relative_axis_map[i].m_axis);
    if (abs(value) > 4000 ) // FIXME: add proper deadzone handling
    {
      m_axis_state[i] += ((m_relative_axis_map[i].m_speed * value) / 32768) * msec_delta / 1000;
      if (m_axis_state[i] < -32768)
        m_axis_state[i] = -32768;
      else if (m_axis_state[i] > 32767)
        m_axis_state[i] = 32767;

      set_axis(msg, m_relative_axis_map[i].m_axis, m_axis_state[i]);
    }
    else
    {
      set_axis(msg, m_relative_axis_map[i].m_axis, m_axis_state[i]);
    }
  }
}

/* EOF */
