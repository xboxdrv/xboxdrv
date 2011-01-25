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

#ifndef HEADER_XBOXDRV_CONTROLLER_SLOT_OPTIONS_HPP
#define HEADER_XBOXDRV_CONTROLLER_SLOT_OPTIONS_HPP

#include <map>
#include <vector>

#include "controller_options.hpp"
#include "controller_match_rule.hpp"

class ControllerSlotOptions
{
public:
  typedef std::map<int, ControllerOptions> Options;

public:
  ControllerSlotOptions();

  void add_match_rule(const ControllerMatchRule& rule);

  ControllerOptions& get_options(int num);
  const ControllerOptions& get_options(int num) const;

  const std::vector<ControllerMatchRule>& get_match_rules() const;
  const std::map<int, ControllerOptions>& get_options() const;

private:
  std::map<int, ControllerOptions> m_options;
  std::vector<ControllerMatchRule> m_match_rules;
};

#endif

/* EOF */
