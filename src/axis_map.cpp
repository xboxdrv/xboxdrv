/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include <logmich/log.hpp>

#include "axis_event_factory.hpp"
#include "controller_message_descriptor.hpp"

AxisMap::AxisMap(const AxisMapOptions& opts, UInput& uinput, int slot, bool extra_devices) :
  m_mappings(),
  m_map()
{
  AxisEventFactory factory(uinput, slot, extra_devices);

  for(AxisMapOptions::const_iterator it = opts.begin(); it != opts.end(); ++it)
  {
    // create event
    AxisEventPtr event = factory.from_string(it->get_event());

    // create filter
    for(std::vector<std::string>::const_iterator j = it->get_filter().begin();
        j != it->get_filter().end();
        ++j)
    {
      event->add_filter(AxisFilter::from_string(*j));
    }

    // event is usable at this point, but can't yet store it in its
    // final place, as the axis strings can't be resolved yet
    Mapping mapping;
    mapping.buttons = it->get_buttons();
    mapping.axis    = it->get_axis();
    mapping.event   = event;
    m_mappings.push_back(mapping);
  }
}

void
AxisMap::init(const ControllerMessageDescriptor& desc)
{
  m_map.clear();
  m_map.resize(desc.get_abs_count());

  for(std::vector<Mapping>::iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
  {
    try
    {
      int abs = desc.abs().get(it->axis);
      m_map.at(abs).add(ButtonCombination(it->buttons), it->event);
    }
    catch(const std::exception& err)
    {
      log_warn("{}: {}", err.what(), it->axis);
    }
  }

  for(int i = 0; i < static_cast<int>(m_map.size()); ++i)
  {
    m_map[i].init(desc);
  }
}

void
AxisMap::send_clear()
{
  for(std::vector<Mapping>::iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
  {
    if (it->event)
    {
      it->event->send(0, -1, +1);
    }
  }
}

void
AxisMap::send(const std::bitset<256>& button_state,
              const std::array<int, 256>& axis_state,
              const std::array<int, 256>& axis_min,
              const std::array<int, 256>& axis_max)
{
  for(int i = 0; i < static_cast<int>(m_map.size()); ++i)
  {
    Map& m = m_map[i];
    m.update(button_state);

    for(Map::iterator j = m.begin(); j != m.end(); ++j)
    {
      if (j->m_data)
      {
        if (j->m_state)
        {
          j->m_data->send(axis_state[i], axis_min[i], axis_max[i]);
        }
        else
        {
          j->m_data->send((axis_max[i] - axis_min[i])/2 + axis_min[i],
                          axis_min[i], axis_max[i]);
        }
      }
    }
  }
}

void
AxisMap::update(int msec_delta)
{
  for(std::vector<Mapping>::iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
  {
    if (it->event)
    {
      it->event->update(msec_delta);
    }
  }
}

/* EOF */
