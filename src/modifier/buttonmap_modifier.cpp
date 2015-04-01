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
#include "key_port.hpp"
#include "raise_exception.hpp"

class ButtonMapping
{
public:
  static ButtonMappingPtr from_string(const std::string& lhs, const std::string& rhs);

public:
  ButtonMapping(const std::string& lhs_str, const std::string& rhs_str) :
    lhs(lhs_str),
    rhs(rhs_str),
    filters()
  {}

  void init(ControllerMessageDescriptor& desc);

  KeyPortIn  lhs;
  KeyPortOut rhs;

  std::vector<ButtonFilterPtr> filters;
};

ButtonMappingPtr
ButtonMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (args.empty())
  {
    raise_exception(std::runtime_error, "empty left hand side");
  }
  else
  {
    ButtonMappingPtr mapping;

    if (rhs.empty())
    {
      mapping.reset(new ButtonMapping(args[0], args[0]));
    }
    else
    {
      mapping.reset(new ButtonMapping(args[0], rhs));
    }

    for(std::vector<std::string>::size_type i = 1; i < args.size(); ++i)
    {
      mapping->filters.push_back(ButtonFilter::from_string(args[i]));
    }

    return mapping;
  }
}

void
ButtonMapping::init(ControllerMessageDescriptor& desc)
{
  lhs.init(desc);
  rhs.init(desc);
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
  for(std::vector<ButtonMappingPtr>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    (*i)->init(desc);
  }
}

void
ButtonmapModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  // update all filters in all mappings
  for(std::vector<ButtonMappingPtr>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    for(std::vector<ButtonFilterPtr>::iterator j = (*i)->filters.begin(); j != (*i)->filters.end(); ++j)
    {
      (*j)->update(msec_delta);
    }
  }

  std::bitset<256> state = msg.get_key_state();

  for(std::vector<ButtonMappingPtr>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    int key_out = (*i)->rhs.get_key();
    bool value  = (*i)->lhs.get(msg);

    // apply the button filter
    for(std::vector<ButtonFilterPtr>::iterator j = (*i)->filters.begin(); j != (*i)->filters.end(); ++j)
    {
      value = (*j)->filter(value);
    }

    state[key_out] = value || state[key_out];
  }

  msg.set_key_state(state);
}

void
ButtonmapModifier::add(ButtonMappingPtr mapping)
{
  m_buttonmap.push_back(mapping);
}

void
ButtonmapModifier::add_filter(const std::string& btn, ButtonFilterPtr filter)
{
  for(std::vector<ButtonMappingPtr>::iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    if ((*i)->lhs.str() == btn)
    {
      (*i)->filters.push_back(filter);
      break;
    }
  }

  // button not already in the map, so add it
  ButtonMappingPtr mapping = ButtonMapping::from_string(btn, btn);
  mapping->filters.push_back(filter);
  add(mapping);
}

std::string
ButtonmapModifier::str() const
{
  std::ostringstream out;
  out << "buttonmap:\n";
  for(std::vector<ButtonMappingPtr>::const_iterator i = m_buttonmap.begin(); i != m_buttonmap.end(); ++i)
  {
    out << "  " << (*i)->lhs.str() << "=" << (*i)->rhs.str() << std::endl;
    for(std::vector<ButtonFilterPtr>::const_iterator filter = (*i)->filters.begin();
        filter != (*i)->filters.end(); ++filter)
    {
      out << "    " << (*filter)->str() << std::endl;
    }
  }
  return out.str();
}

/* EOF */
