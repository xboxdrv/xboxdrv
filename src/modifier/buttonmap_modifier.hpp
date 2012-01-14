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

#ifndef HEADER_XBOXDRV_MODIFIER_BUTTON_MAP_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_BUTTON_MAP_MODIFIER_HPP

#include <vector>

#include "button_filter.hpp"
#include "modifier.hpp"

struct ButtonMappingOption
{
  ButtonMappingOption(const std::string& lhs_,
                      const std::string& rhs_) :
    lhs(lhs_),
    rhs(rhs_)
  {}

  std::string lhs;
  std::string rhs;
};

struct ButtonMapping
{
  static ButtonMapping from_string(const std::string& lhs, const std::string& rhs,
                                   const ControllerMessageDescriptor& msg_desc);

  int lhs;
  int rhs;
  std::vector<ButtonFilterPtr> filters;

  ButtonMapping() :
    lhs(XBOX_BTN_UNKNOWN),
    rhs(XBOX_BTN_UNKNOWN),
    filters()
  {}
};

class ButtonmapModifier : public Modifier
{
public:
  static ButtonmapModifier* from_string(const std::string& args,
                                        const ControllerMessageDescriptor& msg_desc);

  static ButtonmapModifier* from_option(const std::vector<ButtonMappingOption>& mappings,
                                        const ControllerMessageDescriptor& msg_desc);

public:
  ButtonmapModifier();
  
  void update(int msec_delta, ControllerMessage& msg);

  void add(const ButtonMapping& mapping);
  void add_filter(int btn, ButtonFilterPtr filter);

  std::string str() const;

  bool empty() const { return m_buttonmap.empty(); }

public:
  std::vector<ButtonMapping> m_buttonmap;
};

#endif

/* EOF */
