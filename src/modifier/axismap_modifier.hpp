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

#ifndef HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_AXISMAP_MODIFIER_HPP

#include "axis_filter.hpp"
#include "controller_message.hpp"
#include "controller_thread.hpp"
#include "modifier.hpp"

struct AxisMappingOption
{
  AxisMappingOption(const std::string& lhs_,
                    const std::string& rhs_) :
    lhs(lhs_),
    rhs(rhs_)
  {}

  std::string lhs;
  std::string rhs;
};

struct AxisMapping
{
  static AxisMapping from_string(const std::string& lhs, const std::string& rhs);

  std::string lhs_str;
  std::string rhs_str;
  int lhs;
  int rhs;
  bool invert;
  std::vector<AxisFilterPtr> filters;

  AxisMapping() :
    lhs_str(),
    rhs_str(),
    lhs(-1),
    rhs(-1),
    invert(false),
    filters()
  {}

  void init(const ControllerMessageDescriptor& desc);
};

class AxismapModifier : public Modifier 
{
public:
  static AxismapModifier* from_string(const std::string& args);

  static AxismapModifier* from_option(const std::vector<AxisMappingOption>& mappings);

public:
  AxismapModifier();

  void init(ControllerMessageDescriptor& desc);
  void update(int msec_delta, ControllerMessage& msg);

  void add(const AxisMapping& mapping);
  void add_filter(int axis, AxisFilterPtr filter);

  std::string str() const;

  bool empty() const { return m_axismap.empty(); }

public:
  std::vector<AxisMapping> m_axismap;
};

#endif

/* EOF */
