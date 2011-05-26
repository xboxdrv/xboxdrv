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
#include <sstream>

#include "axisfilter/invert_axis_filter.hpp"
#include "helper.hpp"

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

  // clear all lhs values in the newmsg, keep rhs
  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    newmsg.set_axis_float(i->lhs, 0);
  }

  for(std::vector<AxisMapping>::iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    int min = get_axis_min(i->lhs);
    int max = get_axis_max(i->lhs);
    int value = msg.get_axis(i->lhs);

    if (i->invert)
    {
      InvertAxisFilter inv;
      value = inv.filter(value, min, max);
    }

    for(std::vector<AxisFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      value = (*j)->filter(value, min, max);
    }

    float lhs = to_float(value, min, max);

    if (i->lhs == i->rhs)
    {
      newmsg.set_axis_float(i->rhs, lhs);
    }
    else
    {
      // FIXME: this primitive merge kind of works for regular axis,
      // but doesn't work for half axis which have their center at
      // -1.0f
      float rhs = newmsg.get_axis_float(i->rhs);
      newmsg.set_axis_float(i->rhs, Math::clamp(-1.0f, lhs + rhs, 1.0f));
    }
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

std::string
AxismapModifier::str() const
{
  std::ostringstream out;
  out << "axismap:\n";
  for(std::vector<AxisMapping>::const_iterator i = m_axismap.begin(); i != m_axismap.end(); ++i)
  {
    out << "  " << axis2string(i->lhs) << "=" << axis2string(i->rhs) << std::endl;
    for(std::vector<AxisFilterPtr>::const_iterator filter = i->filters.begin(); filter != i->filters.end(); ++filter)
    {
      out << "    " << (*filter)->str() << std::endl;
    }
  }
  return out.str();
}

/* EOF */
