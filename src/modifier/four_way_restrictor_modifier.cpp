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

#include <math.h>
#include <sstream>
#include <stdexcept>

FourWayRestrictorModifier*
FourWayRestrictorModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 2)
  {
    throw std::runtime_error("FourWayRestrictorModifier requires two arguments");
  }
  else
  {
    return new FourWayRestrictorModifier(args[0], args[1]);
  }
}

FourWayRestrictorModifier::FourWayRestrictorModifier(const std::string& xaxis, const std::string& yaxis) :
  m_xaxis_str(xaxis),
  m_yaxis_str(yaxis),
  m_xaxis(-1),
  m_yaxis(-1)
{
}

void
FourWayRestrictorModifier::init(ControllerMessageDescriptor& desc)
{
  m_xaxis = desc.abs().get(m_xaxis_str);
  m_yaxis = desc.abs().get(m_yaxis_str);
}

void
FourWayRestrictorModifier::update(int msec_delta, ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  if (fabsf(msg.get_abs_float(m_xaxis)) > fabsf(msg.get_abs_float(m_yaxis)))
  {
    msg.set_abs_float(m_yaxis, 0);
  }
  else if (fabsf(msg.get_abs_float(m_yaxis)) > fabsf(msg.get_abs_float(m_xaxis)))
  {
    msg.set_abs_float(m_xaxis, 0);
  }
  else
  {
    msg.set_abs_float(m_xaxis, 0);
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
