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

ButtonmapModifier*
ButtonmapModifier::from_string(const std::string& lhs, const std::string& rhs)
{
  std::auto_ptr<ButtonmapModifier> mapping(new ButtonmapModifier(XBOX_BTN_UNKNOWN, XBOX_BTN_UNKNOWN));

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping->m_lhs = string2btn(*t); break;
      default: mapping->m_filters.push_back(ButtonFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping->m_rhs = mapping->m_lhs;
  }
  else
  {
    mapping->m_rhs = string2btn(rhs);
  }

  return mapping.release();
}

ButtonmapModifier::ButtonmapModifier(XboxButton lhs,
                                     XboxButton rhs) :
  m_lhs(lhs),
  m_rhs(rhs)
{
}
  
void
ButtonmapModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  XboxGenericMsg newmsg = msg;

  // update all filters in all mappings
  for(std::vector<ButtonFilterPtr>::iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  // set all buttons to 0
  set_button(newmsg, m_lhs, 0);


  // Take both lhs and rhs into account to allow multiple buttons
  // mapping to the same button
  bool value = get_button(msg, m_lhs);

  // apply the button filter
  for(std::vector<ButtonFilterPtr>::iterator j = m_filters.begin(); j != m_filters.end(); ++j)
  {
    value = (*j)->filter(value);
  }    

  set_button(newmsg, m_rhs, value || get_button(newmsg, m_rhs));

  msg = newmsg;
}

/* EOF */
