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

#ifndef HEADER_XBOXDRV_UINPUT_CFG_HPP
#define HEADER_XBOXDRV_UINPUT_CFG_HPP

#include <vector>

#include "axis_event.hpp"
#include "button_event.hpp"
#include "button_map.hpp"
#include "axis_map.hpp"
#include "xboxmsg.hpp"

class UInputOptions
{
public:
  std::string device_name;

  bool force_feedback;
  XboxButton config_toggle_button; 

private:
  struct InputMapping
  {
    ButtonMap btn_map;
    AxisMap   axis_map;
  };

  std::vector<InputMapping> map;
  int current_input_map;
  
public:
  /** \addtogroup creation Creation Functions
   ** @{*/
  UInputOptions();

  void add_input_mapping();
  void next_input_mapping();
    
  /** Sets a button/axis mapping that is equal to the xpad kernel driver */
  void mimic_xpad();
  void mouse();
  void guitar();
  void set_defaults();

  void trigger_as_button();
  void trigger_as_zaxis();

  void dpad_as_button();
  void dpad_only();

  void set_ui_buttonmap(const std::string& name, const std::string& value);
  void set_ui_axismap(const std::string& name, const std::string& value);
  /** @}*/

  /** \addtogroup access Access Functions
   ** @{*/
  ButtonMap& get_btn_map();
  AxisMap&   get_axis_map();

  ButtonMap& get_btn_map(int n);
  AxisMap&   get_axis_map(int n);
  int input_mapping_count() const { return static_cast<int>(map.size()); }
  /** @}*/
};

#endif

/* EOF */
