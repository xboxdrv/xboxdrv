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

#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>

#include "helper.hpp"

ButtonMapping 
ButtonMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  ButtonMapping mapping;

  mapping.lhs_str.clear();
  mapping.rhs_str.clear();

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.lhs_str = *t; break;
      default: mapping.filters.push_back(ButtonFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.rhs_str = mapping.lhs_str;
  }
  else
  {
    mapping.rhs_str = rhs;
  }

  return mapping;
}

void
ButtonMapping::init(const ControllerMessageDescriptor& desc)
{
  lhs = desc.key().get(lhs_str);
  rhs = desc.key().get(rhs_str);
}

ButtonmapModifier*
ButtonmapModifier::from_string(const std::string& args)
{
  std::auto_ptr<ButtonmapModifier> modifier(new ButtonmapModifier);

  process_name_value_string(args, boost::bind(&ButtonmapModifier::add, modifier.get(), 
                                              boost::bind(&ButtonMapping::from_string, _1, _2)));

  return modifier.release();
}

ButtonmapModifier*
ButtonmapModifier::from_option(const std::vector<ButtonMappingOption>& mappings)
{
  std::auto_ptr<ButtonmapModifier> modifier(new ButtonmapModifier);
  
  for(std::vector<ButtonMappingOption>::const_iterator i = mappings.begin(); i != mappings.end(); ++i)
  {
    modifier->add(ButtonMapping::from_string(i->lhs, i->rhs));
  }

  return modifier.release();
}

ButtonmapModifier::ButtonmapModifier() :
  m_buttonmap()
{
}

void
ButtonmapModifier::init(ControllerMessageDescriptor& desc)
{
  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    i->init(desc);
  }
}
  
void
ButtonmapModifier::update(int msec_delta, ControllerMessage& msg)
{
  ControllerMessage newmsg = msg;

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
    newmsg.set_key(i->lhs, 0);
  }

  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    // Take both lhs and rhs into account to allow multiple buttons
    // mapping to the same button
    bool value = msg.get_key(i->lhs);

    // apply the button filter
    for(std::vector<ButtonFilterPtr>::iterator j = i->filters.begin(); j != i->filters.end(); ++j)
    {
      value = (*j)->filter(value);
    }    

    newmsg.set_key(i->rhs, value || newmsg.get_key(i->rhs));
  }

  msg = newmsg;  
}

void
ButtonmapModifier::add(const ButtonMapping& mapping)
{
  m_buttonmap.push_back(mapping);
}

void
ButtonmapModifier::add_filter(const std::string& btn, ButtonFilterPtr filter)
{
  for(std::vector<ButtonMapping>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    if (i->lhs_str == btn)
    {
      i->filters.push_back(filter);
      break;
    }
  }

  // button not already in the map, so add it
  ButtonMapping mapping;
  mapping.lhs_str = btn;
  mapping.rhs_str = btn;
  mapping.filters.push_back(filter);
  add(mapping);
}

std::string
ButtonmapModifier::str() const
{
  std::ostringstream out;
  /* BROKEN:
  out << "buttonmap:\n";
  for(std::vector<ButtonMapping>::const_iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    out << "  " << btn2string(i->lhs) << "=" << btn2string(i->rhs) << std::endl;
    for(std::vector<ButtonFilterPtr>::const_iterator filter = i->filters.begin(); filter != i->filters.end(); ++filter)
    {
      out << "    " << (*filter)->str() << std::endl;
    }
  }
  */
  return out.str();
}

/* EOF */
