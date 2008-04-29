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
#include "throttle.hpp"

Throttle::Throttle()
  : value(0)
{
  abs_port_in.push_back(new AbsPortIn("Throttle-In", 0, 0, 
                                      boost::bind(&Throttle::on_abs, this, _1)));
  abs_port_out.push_back(new AbsPortOut("Throttle-Out", 0, 32767));
}

void
Throttle::on_abs(AbsPortOut* port)
{
  
}

void
Throttle::update(float delta)
{ 
  if (abs_port_in[0]->out_port)
    {
      int v = abs_port_in[0]->out_port->get_state();
      if (abs(v) > 5000) // FIXME: Deadzone handling must be different
        {
          value -= v * delta * 2.0f;

          if (value < abs_port_out[0]->min_value)
            value = abs_port_out[0]->min_value;
          else if (value > abs_port_out[0]->max_value)
            value = abs_port_out[0]->max_value;

          abs_port_out[0]->set_state(value);
        }
    }
}

/* EOF */
