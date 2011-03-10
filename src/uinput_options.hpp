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

#include "button_map.hpp"
#include "axis_map.hpp"

class UInputOptions
{
private:
  ButtonMap m_btn_map;
  AxisMap   m_axis_map;
  
public:
  /** \addtogroup creation Creation Functions
   ** @{*/
  UInputOptions();
    
  /** Sets a button/axis mapping that is equal to the xpad kernel driver */
  void mimic_xpad();
  void mimic_xpad_wireless();
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

  const ButtonMap& get_btn_map() const;
  const AxisMap&   get_axis_map() const;
  /** @}*/
};

#endif

/* EOF */
