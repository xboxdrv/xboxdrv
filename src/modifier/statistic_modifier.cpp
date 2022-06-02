/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "statistic_modifier.hpp"

#include <fmt/format.h>
#include <iostream>

#include "../xboxmsg.hpp"

namespace xboxdrv {

StatisticModifier*
StatisticModifier::from_string(std::vector<std::string> const& args)
{
  return new StatisticModifier;
}

StatisticModifier::StatisticModifier() :
  m_button_state(),
  m_press_count()
{
}

StatisticModifier::~StatisticModifier()
{
  print_stats();
}

void
StatisticModifier::init(ControllerMessageDescriptor& desc)
{
  m_button_state.resize(desc.get_key_count());
  m_press_count.resize(desc.get_key_count());
}

void
StatisticModifier::print_stats()
{
  std::cout << "Button Press Statistics\n"
            << "=======================\n\n";

  std::cout << fmt::format("{:12s} | {:5d}", "Name", "Count") << std::endl;
  std::cout << "-------------+---------" << std::endl;
  for(size_t i = 0; i < m_press_count.size(); ++i)
  {
    std::cout << fmt::format("{:12s} : {:5d}\n", i, m_press_count[i]);
  }
}

void
StatisticModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  for(size_t btn = 0; btn < m_press_count.size(); ++btn)
  {
    bool state = msg.get_key(static_cast<int>(btn));

    // state changed and button is pressed
    if (state != m_button_state[btn] && state)
    {
      m_press_count[btn] += 1;
    }

    m_button_state[btn] = state;
  }
}

std::string
StatisticModifier::str() const
{
  return "stat";
}

} // namespace xboxdrv

/* EOF */
