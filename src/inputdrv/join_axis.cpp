/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "join_axis.hpp"

#include <boost/bind.hpp>

#include "abs_port_in.hpp"
#include "abs_port_out.hpp"
#include "btn_port_in.hpp"
#include "btn_port_out.hpp"
#include "rel_port_in.hpp"
#include "rel_port_out.hpp"

JoinAxis::JoinAxis()
{
  abs_port_in.push_back(new AbsPortIn("JoinAxis-1", 0, 0, 
                                      boost::bind(&JoinAxis::on_abs, this, _1)));
  abs_port_in.push_back(new AbsPortIn("JoinAxis-2", 0, 0, 
                                      boost::bind(&JoinAxis::on_abs, this, _1)));

  // FIXME: Abs handling must do something proper with the min/max values
  abs_port_out.push_back(new AbsPortOut("JoinAxis-Out-1", -255, 255));
}

void
JoinAxis::on_abs(AbsPortOut* port)
{
  int value = 0;

  value -= abs_port_in[0]->out_port->get_state();
  value += abs_port_in[1]->out_port->get_state();
  
  // clamp
  value = std::max(abs_port_out[0]->min_value, 
                   std::min(value,
                            abs_port_out[0]->max_value));
  
  abs_port_out[0]->set_state(value);
}

/* EOF */
