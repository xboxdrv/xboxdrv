/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_MODIFIER_SECTOR2BUTTON_MODIFIER_HPP
#define HEADER_XBOXDRV_MODIFIER_SECTOR2BUTTON_MODIFIER_HPP

#include <vector>
#include <numeric>

#include "modifier.hpp"

namespace xboxdrv {

class Sector2ButtonModifier : public Modifier
{
public:
  static Sector2ButtonModifier* from_string(const std::vector<std::string>& args);

public:
  Sector2ButtonModifier(const std::string& xaxis, const std::string& yaxis,
                        const std::vector<std::string>& out_buttons_str);

  void init(ControllerMessageDescriptor& desc) override;
  void update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc) override;

  std::string str() const override;

private:
  std::string m_xaxis_str;
  std::string m_yaxis_str;

  std::vector<std::string> m_out_buttons_str;

  int m_xaxis;
  int m_yaxis;
  std::vector<int> m_out_buttons;

private:
  Sector2ButtonModifier(const Sector2ButtonModifier&);
  Sector2ButtonModifier& operator=(const Sector2ButtonModifier&);
};

} // namespace xboxdrv

#endif

/* EOF */
