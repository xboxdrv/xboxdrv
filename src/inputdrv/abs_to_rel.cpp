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

#include <boost/bind.hpp>
#include "abs_to_rel.hpp"

AbsToRel::AbsToRel()
{
  abs_port_in.push_back(new AbsPortIn("AbsToRel-In", 0, 0, 
                                      boost::bind(&AbsToRel::on_abs, this, _1)));
  rel_port_out.push_back(new RelPortOut("AbsToRel-Out"));
}

void
AbsToRel::on_abs(AbsPortOut* port)
{
  // Nothing do to, since everything is handled in update
  //rel_port_out[0]->set_state(port->get_state());
}

void
AbsToRel::update(float delta)
{
  int s = int(abs_port_in[0]->out_port->get_state() * delta * 0.2f);
  rel_port_out[0]->set_state(s);
}

/* EOF */
