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

#include "sector2button_modifier.hpp"

#include <stdexcept>
#include <math.h>

#include "raise_exception.hpp"

Sector2ButtonModifier*
Sector2ButtonModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 10)
  {
    raise_exception(std::runtime_error, "incorrect number of arguments, need 10");
  }
  else
  {
    return new Sector2ButtonModifier(args[0], args[1],
                                     std::vector<std::string>(args.begin()+2, args.end()));
  }
}

Sector2ButtonModifier::Sector2ButtonModifier(const std::string& xaxis, const std::string& yaxis,
                                             const std::vector<std::string>& out_buttons_str) :
  m_xaxis_str(xaxis),
  m_yaxis_str(yaxis),
  m_out_buttons_str(out_buttons_str),
  m_xaxis(-1),
  m_yaxis(-1),
  m_out_buttons()
{
}

void
Sector2ButtonModifier::init(ControllerMessageDescriptor& desc)
{
  m_xaxis = desc.abs().get(m_xaxis_str);
  m_yaxis = desc.abs().get(m_yaxis_str);

  m_out_buttons.clear();
  for(std::vector<std::string>::const_iterator it = m_out_buttons_str.begin(); it != m_out_buttons_str.end(); ++it)
  {
    m_out_buttons.push_back(desc.key().getput(*it));
  }
}

void
Sector2ButtonModifier::update(int msec_delta, ControllerMessage& msg)
{
  float x = msg.get_abs_float(m_xaxis);
  float y = msg.get_abs_float(m_yaxis);
  
  if (x != 0 || y != 0)
  {
    double sector_size = static_cast<double>(2*M_PI) / static_cast<double>(m_out_buttons.size());

    // FIXME: fugly angle hackery
    double length = sqrt(x*x + y*y);
    double angle  = atan2(y, x) + M_PI/2.0;
    angle += sector_size/2.0;
    angle = fmod(angle + 4*M_PI, 2*M_PI);

    if (length > 0.75f) // FIXME: hardcoded threshold
    {
      int sector = static_cast<int>((angle) / sector_size);
      //log_tmp("sector: " << sector << " " << angle);
      msg.set_key(m_out_buttons[sector], true);
    }
  }
}
  
std::string
Sector2ButtonModifier::str() const
{
  return std::string();
}

/* EOF */
