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

#include "toggle_button.hpp"

#include <boost/bind.hpp>

#include "abs_port_in.hpp"
#include "abs_port_out.hpp"
#include "btn_port_in.hpp"
#include "btn_port_out.hpp"
#include "rel_port_in.hpp"
#include "rel_port_out.hpp"

ToggleButton::ToggleButton()
  : state(false)
{
  btn_port_out.push_back(new BtnPortOut("ToggleButtonIn(out)"));
  btn_port_in.push_back(new BtnPortIn("ToggleButton(in)",    
                                      boost::bind(&ToggleButton::on_btn, this, _1))); 
}

ToggleButton::~ToggleButton()
{
}

void
ToggleButton::on_btn(BtnPortOut* out)
{
  if (out->get_state())
    {
      state = !state;
      btn_port_out[0]->set_state(state);
    }
}

/* EOF */
