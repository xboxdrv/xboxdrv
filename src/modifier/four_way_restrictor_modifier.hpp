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

#ifndef HEADER_XBOXDRV_MODIFIER_FOUR_WAY_RESTRICTOR_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_FOUR_WAY_RESTRICTOR_MODIFIER_HPP

#include <vector>

#include "abs_port.hpp"
#include "modifier.hpp"

namespace xboxdrv {

class FourWayRestrictorModifier : public Modifier
{
public:
  static FourWayRestrictorModifier* from_string(const std::vector<std::string>& args);

public:
  FourWayRestrictorModifier(const std::string& xaxis_in, const std::string& yaxis_in,
                            const std::string& xaxis_out, const std::string& yaxis_out);

  void init(ControllerMessageDescriptor& desc) override;
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc) override;

  std::string str() const override;

private:
  AbsPortIn m_xaxis_in;
  AbsPortIn m_yaxis_in;

  AbsPortOut m_xaxis_out;
  AbsPortOut m_yaxis_out;
};

} // namespace xboxdrv

#endif

/* EOF */
