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

#include "modifier/stick_zone_modifier.hpp"

#include <math.h>
#include <sstream>
#include <stdexcept>

#include "util/string.hpp"
#include "xboxmsg.hpp"
#include "raise_exception.hpp"

namespace xboxdrv {

StickZoneModifier*
StickZoneModifier::from_string(std::vector<std::string> const& args)
{
  if (args.size() != 5)
  {
    raise_exception(std::runtime_error, "StickZoneModifier requires five arguments");
  }
  else
  {
    return new StickZoneModifier(args[0], args[1], args[2],
                                 str2float(args[3]),
                                 str2float(args[4]));
  }
}

StickZoneModifier::StickZoneModifier(std::string const& x_axis, std::string const& y_axis,
                                     std::string const& button,
                                     float range_start, float range_end) :

  m_x_axis(x_axis),
  m_y_axis(y_axis),
  m_button(button),
  m_range_start(range_start),
  m_range_end(range_end)
{
}

void
StickZoneModifier::init(ControllerMessageDescriptor& desc)
{
  m_x_axis.init(desc);
  m_y_axis.init(desc);
  m_button.init(desc);
}

void
StickZoneModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  float x = m_x_axis.get_float(msg);
  float y = m_y_axis.get_float(msg);
  float r = sqrtf(x*x + y*y);

  if (r > 1.0f)
  {
    r = 1.0f;
  }

  if (m_range_start <= r && r <= m_range_end)
  {
    m_button.set(msg, true);
  }
  else
  {
    m_button.set(msg, false);
  }
}

std::string
StickZoneModifier::str() const
{
  std::ostringstream os;
  os << "stickzone:" << m_x_axis.str() << ":" << m_y_axis.str() << ":" << m_button.str();
  return os.str();
}

} // namespace xboxdrv

/* EOF */
