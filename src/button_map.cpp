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

ButtonMap::ButtonMap() :
  m_mappings()
{
}

void
ButtonMap::bind(const ButtonCombination& buttons, ButtonEventPtr event)
{
  // FIXME: binding the same combo twice might lead to problems
  Mapping mapping(buttons, event);

  // find of which already bound combinations the new one is a
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
  assert(!"implement me");
  return ButtonEventPtr();
}

void
ButtonMap::clear()
{
  m_mappings.clear();
}

void
ButtonMap::init(UInput& uinput, int slot, bool extra_devices)
{
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->init(uinput, slot, extra_devices);
  }
}

void
ButtonMap::send(UInput& uinput, const std::bitset<XBOX_BTN_MAX>& button_state)
{
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    if (i->m_buttons.match(button_state))
    {
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
        i->m_event->send(uinput, false);
      }
      else
      {
        i->m_event->send(uinput, true);
      }
    }
    else
    {
      i->m_event->send(uinput, false);
    }
  }
}

void
ButtonMap::send_clear(UInput& uinput)
{
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->send_clear(uinput);
  }
}

void
ButtonMap::update(UInput& uinput, int msec_delta)
{
  for(Mappings::const_iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->update(uinput, msec_delta);
  }
}

/* EOF */
