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
#include <memory>

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

  mapping.invert = false;
  mapping.lhs = XBOX_AXIS_UNKNOWN;
  mapping.rhs = XBOX_AXIS_UNKNOWN;

  if (lhs[0] == '-')
  {
    mapping.invert = true;
    lhs = lhs.substr(1);
  }

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.lhs = string2axis(*t); break;
      default: mapping.filters.push_back(AxisFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.rhs = mapping.lhs;
  }
  else
  {
    mapping.rhs = string2axis(rhs);
  }

  if (mapping.lhs == XBOX_AXIS_UNKNOWN ||
      mapping.rhs == XBOX_AXIS_UNKNOWN)
  {
    throw std::runtime_error("Couldn't convert string \"" + lhs + "=" + rhs + "\" to axis mapping");
  }

  return mapping;
}

AxismapModifier::AxismapModifier() :
  m_axismap()
{
}

void
AxismapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  // update all filters in all mappings
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    for(std::vector<AxisFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      (*j)->update(msec_delta);
    }
  }

  // clear all values in the new msg
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    set_axis_float(newmsg, i->lhs, 0);
  }

  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    int min = get_axis_min(i->lhs);
    int max = get_axis_max(i->lhs);
    int value = get_axis(msg, i->lhs);

    for(std::vector<AxisFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      value = (*j)->filter(value, min, max);
    }

    float lhs  = to_float(value, min, max);
    float nrhs = get_axis_float(newmsg, i->rhs);

    if (i->invert)
    {
      lhs = -lhs;
    }

    set_axis_float(newmsg, i->rhs, std::max(std::min(nrhs + lhs, 1.0f), -1.0f));
  }
  msg = newmsg;
}

void
AxismapModifier::add(const AxisMapping& mapping)
{
  m_axismap.push_back(mapping);
}

void
AxismapModifier::add_filter(XboxAxis axis, AxisFilterPtr filter)
{
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    if (i->lhs == axis)
    {
      i->filters.push_back(filter);
      break;
    }
  }

  AxisMapping mapping;
  mapping.lhs = axis;
  mapping.rhs = axis;
  mapping.invert = false;
  mapping.filters.push_back(filter);
  add(mapping);
}

/* EOF */
