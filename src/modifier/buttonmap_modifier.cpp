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

#include "buttonmap_modifier.hpp"

#include <boost/tokenizer.hpp>
#include <sstream>

ButtonMapping 
ButtonMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  ButtonMapping mapping;

  mapping.lhs = XBOX_BTN_UNKNOWN;
  mapping.rhs = XBOX_BTN_UNKNOWN;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.lhs = string2btn(*t); break;
      default: mapping.filters.push_back(ButtonFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.rhs = mapping.lhs;
  }
  else
  {
    mapping.rhs = string2btn(rhs);
  }

  return mapping;
}

ButtonmapModifier::ButtonmapModifier() :
  m_buttonmap()
{
}
  
void
ButtonmapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  // update all filters in all mappings
  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    for(std::vector<ButtonFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      (*j)->update(msec_delta);
    }
  }

  // set all buttons to 0
  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    set_button(newmsg, i->lhs, 0);
  }

  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    // Take both lhs and rhs into account to allow multiple buttons
    // mapping to the same button
    bool value = get_button(msg, i->lhs);

    // apply the button filter
    for(std::vector<ButtonFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      value = (*j)->filter(value);
    }    

    set_button(newmsg, i->rhs, value || get_button(newmsg, i->rhs));
  }

  msg = newmsg;  
}

void
ButtonmapModifier::add(const ButtonMapping& mapping)
{
  m_buttonmap.push_back(mapping);
}

void
ButtonmapModifier::add_filter(XboxButton btn, ButtonFilterPtr filter)
{
  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    if (i->lhs == btn)
    {
      i->filters.push_back(filter);
      break;
    }
  }

  // button not already in the map, so add it
  ButtonMapping mapping;
  mapping.lhs = btn;
  mapping.rhs = btn;
  mapping.filters.push_back(filter);
  add(mapping);
}

std::string
ButtonmapModifier::str() const
{
  std::ostringstream out;
  out << "buttonmap:";
  for(std::vector<ButtonMapping>::const_iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    out << btn2string(i->lhs) << "=" << btn2string(i->rhs) << std::endl;
    for(std::vector<ButtonFilterPtr>::const_iterator filter = i->filters.begin(); filter != i->filters.end(); ++filter)
    {
      out << "  " << (*filter)->str() << std::endl;
    }
  }
  return out.str();
}

/* EOF */
