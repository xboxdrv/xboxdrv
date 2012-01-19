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

ButtonMap::ButtonMap(const ButtonMapOptions& opts, UInput& uinput, int slot, bool extra_devices) :
  m_map()
{
  ButtonEventFactory button_event_factory(uinput, slot, extra_devices);

  // BROKEN: Events must not be overriden after creation, as that messes up UInput
  for(ButtonMapOptions::const_iterator it = opts.begin(); it != opts.end(); ++it)
  {
    ButtonCombination buttons = ButtonCombination::from_string(it->get_button());

    ButtonEventPtr event;
    if (it->get_event().empty())
    { 
#if 0
      // BROKEN
      // if no new event is given, add filters to the current binding
      event = lookup(buttons);
#endif
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
ButtonMap::init(const ControllerMessageDescriptor& desc)
{
  m_map.init(desc);
}

void
ButtonMap::bind(const ButtonCombination& buttons, ButtonEventPtr event)
{
  m_map.add(buttons, event);
}

void
ButtonMap::send(const std::bitset<256>& button_state)
{
  m_map.update(button_state);

  for(Map::iterator i = m_map.begin(); i != m_map.end(); ++i)
  {
    i->m_data->send(i->m_state);
  }
}

void
ButtonMap::send_clear()
{
  for(Map::iterator i = m_map.begin(); i != m_map.end(); ++i)
  {
    i->m_data->send_clear();
  }
}

void
ButtonMap::update(int msec_delta)
{
  for(Map::const_iterator i = m_map.begin(); i != m_map.end(); ++i)
  {
    i->m_data->update(msec_delta);
  }
}

/* EOF */
