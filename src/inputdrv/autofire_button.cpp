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

#include "autofire_button.hpp"

#include <boost/bind.hpp>

#include "btn_port_in.hpp"
#include "btn_port_out.hpp"

AutofireButton::AutofireButton(int rate)
  : rate(rate),
    rate_counter(0)
{
  btn_port_in.push_back(new BtnPortIn("AutofireButton-In", 
                                     boost::bind(&AutofireButton::on_btn, this, _1)));
  btn_port_out.push_back(new BtnPortOut("AutofireButton-Out-0")); 
}

void
AutofireButton::on_btn(BtnPortOut* port)
{
  btn_port_out[0]->set_state(port->get_state());
}

void
AutofireButton::update(float delta)
{
  if (btn_port_in[0]->out_port->get_state())
    {
      rate_counter += int(1000 * delta);
      if (rate_counter >= rate)
        {
          rate_counter = rate_counter % rate;
          btn_port_out[0]->set_state(!btn_port_out[0]->get_state());
        }
    }
  else
    {
      rate_counter = 0;
    }
}

/* EOF */
