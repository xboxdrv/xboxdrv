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

#include "axismap_modifier.hpp"

#include <boost/tokenizer.hpp>

/** converts the arbitary range to [-1,1] */
inline float to_float(int value, int min, int max)
{
  return static_cast<float>(value - min) / static_cast<float>(max - min) * 2.0f - 1.0f;
}

AxisMapping
AxisMapping::from_string(const std::string& lhs_, const std::string& rhs)
{
  std::string lhs = lhs_;
  AxisMapping mapping;

  mapping.m_invert = false;
  mapping.m_lhs = XBOX_AXIS_UNKNOWN;
  mapping.m_rhs = XBOX_AXIS_UNKNOWN;

  if (lhs[0] == '-')
  {
    mapping.m_invert = true;
    lhs = lhs.substr(1);
  }

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.m_lhs = string2axis(*t); break;
      default: mapping.m_filters.push_back(AxisFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.m_rhs = mapping.m_lhs;
  }
  else
  {
    mapping.m_rhs = string2axis(rhs);
  }

  if (mapping.m_lhs == XBOX_AXIS_UNKNOWN ||
      mapping.m_rhs == XBOX_AXIS_UNKNOWN)
  {
    throw std::runtime_error("Couldn't convert string \"" + lhs + "=" + rhs + "\" to axis mapping");
  }

  return mapping;
}

AxismapModifier::AxismapModifier(const std::vector<AxisMapping>& axismap) :
  m_axismap(axismap)
{
}

void
AxismapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  // update all filters in all mappings
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    for(std::vector<AxisFilterPtr>::iterator j = i->m_filters.begin(); j != i->m_filters.end(); ++j)
    {
      (*j)->update(msec_delta);
    }
  }

  // clear all values in the new msg
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    set_axis_float(newmsg, i->m_lhs, 0);
  }

  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    int min = get_axis_min(i->m_lhs);
    int max = get_axis_max(i->m_lhs);
    int value = get_axis(msg, i->m_lhs);

    for(std::vector<AxisFilterPtr>::iterator j = i->m_filters.begin(); j != i->m_filters.end(); ++j)
    {
      value = (*j)->filter(value, min, max);
    }

    float lhs  = to_float(value, min, max);
    float nrhs = get_axis_float(newmsg, i->m_rhs);

    if (i->m_invert)
    {
      lhs = -lhs;
    }

    set_axis_float(newmsg, i->m_rhs, std::max(std::min(nrhs + lhs, 1.0f), -1.0f));
  }
  msg = newmsg;
}

/* EOF */
