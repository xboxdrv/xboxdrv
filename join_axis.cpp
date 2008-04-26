/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <boost/bind.hpp>
#include "join_axis.hpp"

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
