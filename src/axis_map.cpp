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

#include "axis_map.hpp"

#include "axis_event_factory.hpp"
#include "controller_message_descriptor.hpp"

AxisMap::AxisMap(const AxisMapOptions& opts, UInput& uinput, int slot, bool extra_devices) :
  m_axis_map()
{
  AxisEventFactory button_event_factory(uinput, slot, extra_devices);
}

void
AxisMap::init(const ControllerMessageDescriptor& desc)
{
}

void
AxisMap::bind(AxisEventPtr event)
{
}

void
AxisMap::bind(const ButtonCombination& combo, AxisEventPtr event)
{
}

void
AxisMap::send_clear()
{
#if 0
  for(Mappings::iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
  {
    i->m_event->send_clear();
  }
#endif
}

void
AxisMap::send(const std::bitset<256>& button_state,
              const boost::array<int, 256>& axis_state)
{
  for(AxisMapping::iterator i = m_axis_map.begin(); i != m_axis_map.end(); ++i)
  {
#if 0
    i->update(button_state);
    for(auto it = i->begin(); it != i->end(); ++i)
    {
      if (i->is_active())
      {
        i->get_data()->send(axis, axis_state[axis]); 
      }
      else
      {
        i->get_data()->send(0);
      }
    }
#endif
  }
}

void
AxisMap::update(int msec_delta)
{
#if 0
  for(size_t shift_code = 0; shift_code < m_axis_map.size(); ++shift_code)
  {
    for(size_t code = 0; code < m_axis_map[shift_code].size(); ++code)
    {
      if (m_axis_map[shift_code][code])
      {
        m_axis_map[shift_code][code]->update(uinput, msec_delta);
      }
    }
  }
#endif
}

/* EOF */
