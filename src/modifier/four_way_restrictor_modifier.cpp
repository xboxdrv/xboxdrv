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

#include "four_way_restrictor_modifier.hpp"

#include <stdexcept>
#include <sstream>

FourWayRestrictorModifier*
FourWayRestrictorModifier::from_string(const std::vector<std::string>& args,
                                       const ControllerMessageDescriptor& msg_desc)
{
  if (args.size() != 2)
  {
    throw std::runtime_error("FourWayRestrictorModifier requires two arguments");
  }
  else
  {
    return new FourWayRestrictorModifier(msg_desc.get_abs(args[0]),
                                         msg_desc.get_abs(args[1]));
  }
}

FourWayRestrictorModifier::FourWayRestrictorModifier(int xaxis, int yaxis) :
  m_xaxis(xaxis),
  m_yaxis(yaxis)
{
}

void
FourWayRestrictorModifier::update(int msec_delta, ControllerMessage& msg)
{
  if (abs(msg.get_abs(m_xaxis)) > abs(msg.get_abs(m_yaxis)))
  {
    msg.set_abs(m_yaxis, 0);
  }
  else if (abs(msg.get_abs(m_yaxis)) > abs(msg.get_abs(m_xaxis)))
  {
    msg.set_abs(m_xaxis, 0);
  }
  else
  {
    msg.set_abs(m_xaxis, 0);
  }
}

std::string
FourWayRestrictorModifier::str() const
{
  std::ostringstream out;
  //BROKEN: out << "4way:" << axis2string(m_xaxis) << ":" << axis2string(m_yaxis);
  return out.str();
}

/* EOF */
