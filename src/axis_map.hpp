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

#ifndef HEADER_XBOXDRV_AXIS_MAP_HPP
#define HEADER_XBOXDRV_AXIS_MAP_HPP

#include <array>

#include <uinpp/fwd.hpp>

#include "axis_event.hpp"
#include "axis_map_option.hpp"
#include "button_combination_map.hpp"

namespace xboxdrv {

class ControllerMessageDescriptor;

class AxisMap
{
private:
  struct Mapping
  {
    std::vector<std::string> buttons;
    std::string  axis;
    AxisEventPtr event;

    Mapping() :
      buttons(),
      axis(),
      event()
    {}
  };

  std::vector<Mapping> m_mappings;

  typedef ButtonCombinationMap<AxisEventPtr> Map;
  typedef std::vector<Map> AxisMapping;
  AxisMapping m_map;

public:
  AxisMap(const AxisMapOptions& opts, uinpp::MultiDevice& uinput, int slot, bool extra_devices);

  void init(const ControllerMessageDescriptor& desc);

  void send(const std::bitset<256>& button_state,
            const std::array<int, 256>& axis_state,
            const std::array<int, 256>& axis_min,
            const std::array<int, 256>& axis_max);
  void send_clear();
  void update(int msec_delta);
};

} // namespace xboxdrv

#endif

/* EOF */
