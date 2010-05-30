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

#include "btn_to_abs.hpp"

#include <boost/bind.hpp>

#include "abs_port_in.hpp"
#include "abs_port_out.hpp"
#include "btn_port_in.hpp"
#include "btn_port_out.hpp"

BtnToAbs::BtnToAbs()
  : target_value(0)
{
  btn_port_in.push_back(new BtnPortIn("BtnToAbs-Out-0",
                                      boost::bind(&BtnToAbs::on_btn, this, _1)));
  btn_port_in.push_back(new BtnPortIn("BtnToAbs-Out-1",
                                      boost::bind(&BtnToAbs::on_btn, this, _1)));

  abs_port_out.push_back(new AbsPortOut("BtnToAbs-In", -32767, 32767));
}

void
BtnToAbs::update(float delta)
{
  // make this configurable
  float relax = 0.05f;
  abs_port_out[0]->set_state(int(abs_port_out[0]->get_state() + 
                                 relax * (target_value - abs_port_out[0]->get_state())));
}

void
BtnToAbs::on_btn(BtnPortOut* port)
{
  if (btn_port_in[0]->out_port->get_state() &&
      !btn_port_in[1]->out_port->get_state())
    {
      target_value = abs_port_out[0]->min_value;
    } 
  else if (!btn_port_in[0]->out_port->get_state() &&
           btn_port_in[1]->out_port->get_state())
    {
      target_value = abs_port_out[0]->max_value;
    }
  else
    {
      target_value = 0;
    }
}

/* EOF */
