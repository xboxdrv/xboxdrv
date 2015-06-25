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

#ifndef HEADER_XBOXDRV_BUTTON_MAP_HPP
#define HEADER_XBOXDRV_BUTTON_MAP_HPP

#include <bitset>
#include <map>
#include <vector>

#include "button_combination.hpp"
#include "button_combination_map.hpp"
#include "button_event.hpp"
#include "button_map_option.hpp"

class UInput;

class ButtonMap
{
private:
  typedef ButtonCombinationMap<ButtonEventPtr> Map;
  Map m_map;

public:
  ButtonMap(const ButtonMapOptions& opts, UInput& uinput, int slot, bool extra_devices);

  void init(const ControllerMessageDescriptor& desc);

  void send(const std::bitset<256>& button_state);
  void send_clear();
  void update(int msec_delta);

private:
  /** Bind a combination of multiple buttons to an event (i.e. "LB+A=KEY_A") */
  void bind(const ButtonCombination& buttons, ButtonEventPtr event);
};

#endif

/* EOF */
