/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "button_map.hpp"

#include <iostream>

#include "button_event_factory.hpp"

ButtonMap::ButtonMap() :
  m_mappings()
{
}

void
ButtonMap::init(const ControllerMessageDescriptor& desc)
{
  std::cout << "ButtonMap::init(const ControllerMessageDescriptor& desc): " << m_mappings.size() << std::endl;
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_buttons.init(desc);

    // FIXME: double init is not very pretty
    for(std::vector<ButtonCombination>::iterator j = i->m_supersets.begin(); j != i->m_supersets.end(); ++j)
    {
      j->init(desc);
    }

    std::cout << "Buttons: ";
    i->m_buttons.print(std::cout);
    std::cout << std::endl;
  }
}

void
ButtonMap::bind(const ButtonCombination& buttons, ButtonEventPtr event)
{
  buttons.print(std::cout);

  // FIXME: binding the same combo twice might lead to problems
  Mapping mapping(buttons, event);

  // find which already bound combinations the new one is a
  // superset of and add it to the list
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    if (i->m_buttons.is_subset_of(buttons))
    {
      i->m_supersets.push_back(buttons);
    }

    if (buttons.is_subset_of(i->m_buttons))
    {
      mapping.m_supersets.push_back(i->m_buttons);
    }
  }

  m_mappings.push_back(mapping);
}

ButtonEventPtr
ButtonMap::lookup(const ButtonCombination& buttons) const
{
  for(Mappings::const_iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    if (i->m_buttons == buttons)
    {
      return i->m_event;
    }
  }
  return ButtonEventPtr();
}

void
ButtonMap::clear()
{
  m_mappings.clear();
}

void
ButtonMap::init(const ButtonMapOptions& opts, UInput& uinput, int slot, bool extra_devices)
{
  ButtonEventFactory button_event_factory(uinput, slot, extra_devices);

  // BROKEN: Events must not be overriden after creation, as that messes up UInput
  for(ButtonMapOptions::const_iterator it = opts.begin(); it != opts.end(); ++it)
  {
    ButtonCombination buttons = ButtonCombination::from_string(it->get_button());

    ButtonEventPtr event;
    if (it->get_event().empty())
    { 
      // if no new event is given, add filters to the current binding
      event = lookup(buttons);
    }
    else
    {
      event = button_event_factory.from_string(it->get_event(), it->get_directory()); 
      if (event)
      {
        bind(buttons, event);
      }
    }

    // FIXME: How are unbound events handled?!
    for(std::vector<std::string>::const_iterator j = it->get_filter().begin(); 
        j != it->get_filter().end();
        ++j)
    {
      if (event)
      {
        event->add_filter(ButtonFilter::from_string(*j));
      }
    }
  }
}

void
ButtonMap::send(const std::bitset<256>& button_state)
{
  //std::cout << "ButtonMap::send" << std::endl;
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    if (i->m_buttons.match(button_state))
    {
      //std::cout << "  match: ";
      //i->m_buttons.print(std::cout);
      //std::cout << std::endl;

      // check if a superset matches
      bool superset_matches = false;
      for(std::vector<ButtonCombination>::iterator j = i->m_supersets.begin(); j != i->m_supersets.end(); ++j)
      {      
        if (j->match(button_state))
        {
          superset_matches = true;
          break;
        }
      }     

      if (superset_matches)
      {
        i->m_event->send(false);
      }
      else
      {
        i->m_event->send(true);
      }
    }
    else
    {
      i->m_event->send(false);
    }
  }
}

void
ButtonMap::send_clear()
{
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->send_clear();
  }
}

void
ButtonMap::update(int msec_delta)
{
  for(Mappings::const_iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->update(msec_delta);
  }
}

/* EOF */
