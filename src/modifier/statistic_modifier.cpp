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

#include "statistic_modifier.hpp"

#include <boost/format.hpp>
#include <iostream>

#include "../xboxmsg.hpp"

StatisticModifier*
StatisticModifier::from_string(const std::vector<std::string>& args)
{
  return new StatisticModifier;
}

StatisticModifier::StatisticModifier() :
  m_button_state(XBOX_BTN_MAX),
  m_press_count(XBOX_BTN_MAX)
{
}

StatisticModifier::~StatisticModifier()
{
  print_stats();
}

void
StatisticModifier::print_stats()
{
  std::cout << "Button Press Statistics\n"
            << "=======================\n\n";
  
  std::cout << boost::format("%12s | %5d") % "Name" % "Count" << std::endl;  
  std::cout << "-------------+---------" << std::endl;
  for(int btn = 1; btn < XBOX_BTN_MAX; ++btn)
  {
    std::cout << boost::format("%12s : %5d") 
      % btn2string(static_cast<XboxButton>(btn)) % m_press_count[btn]
              << std::endl;
  }
}

void
StatisticModifier::update(int msec_delta, ControllerMessage& msg)
{
  for(int btn = 1; btn < static_cast<int>(XBOX_BTN_MAX); ++btn)
  {
    bool state = msg.get_button(static_cast<XboxButton>(btn));

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

/* EOF */
