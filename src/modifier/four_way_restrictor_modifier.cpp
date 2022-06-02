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

#include "four_way_restrictor_modifier.hpp"

#include <math.h>
#include <sstream>
#include <stdexcept>

namespace xboxdrv {

FourWayRestrictorModifier*
FourWayRestrictorModifier::from_string(std::vector<std::string> const& args)
{
  if (args.size() == 2)
  {
    return new FourWayRestrictorModifier(args[0], args[1], args[0], args[1]);
  }
  else if (args.size() == 4)
  {
    return new FourWayRestrictorModifier(args[0], args[1], args[2], args[3]);
  }
  else
  {
    throw std::runtime_error("FourWayRestrictorModifier requires two or four arguments");
  }
}

FourWayRestrictorModifier::FourWayRestrictorModifier(std::string const& xaxis_in, std::string const& yaxis_in,
                                                     std::string const& xaxis_out, std::string const& yaxis_out) :
  m_xaxis_in(xaxis_in),
  m_yaxis_in(yaxis_in),
  m_xaxis_out(xaxis_out),
  m_yaxis_out(yaxis_out)
{
}

void
FourWayRestrictorModifier::init(ControllerMessageDescriptor& desc)
{
  m_xaxis_in.init(desc);
  m_yaxis_in.init(desc);
  m_xaxis_out.init(desc);
  m_yaxis_out.init(desc);
}

void
FourWayRestrictorModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  if (fabsf(m_xaxis_in.get_float(msg)) > fabsf(m_yaxis_in.get_float(msg)))
  {
    m_xaxis_out.set_float(msg, m_xaxis_in.get_float(msg));
    m_yaxis_out.set_float(msg, 0);
  }
  else
  {
    m_xaxis_out.set_float(msg, 0);
    m_yaxis_out.set_float(msg, m_yaxis_in.get_float(msg));
  }
}

std::string
FourWayRestrictorModifier::str() const
{
  std::ostringstream out;
  out << "4way:"
      << m_xaxis_in.str()  << ":" << m_yaxis_in.str() << ":"
      << m_xaxis_out.str() << ":" << m_yaxis_out.str();
  return out.str();
}

} // namespace xboxdrv

/* EOF */
