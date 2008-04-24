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

#include <usb.h>
#include <iostream>
#include <boost/bind.hpp>
#include "xbox360_driver.hpp"
#include "toggle_button.hpp"
#include "control.hpp"
#include "inputdrv.hpp"

void btn_change(BtnPortOut* port)
{
  std::cout << "Button: " << port->get_label() << ": " << port->get_state() << std::endl;
}

void abs_change(AbsPortOut* port)
{
  std::cout << "Axis:   " << port->get_label() << ": " << port->get_state() << std::endl;
}

int main()
{
  // Init USB
  usb_init();
  usb_find_busses();
  usb_find_devices();
  
  Xbox360Driver xbox360(0);
  ToggleButton  toggle;

  BtnPortOut* btn_a = xbox360.get_btn_port_out(Xbox360Driver::XBOX360_BTN_A);
  BtnPortOut* btn_b = xbox360.get_btn_port_out(Xbox360Driver::XBOX360_BTN_B);
  BtnPortIn*  toggle_in  = toggle.get_btn_port_in(0);
  BtnPortOut* toggle_out = toggle.get_btn_port_out(0);

  btn_a->connect(btn_change);
  btn_b->connect(toggle_in);

  toggle_out->connect(xbox360.get_btn_port_in(0));

  xbox360.get_abs_port_out(Xbox360Driver::XBOX360_AXIS_LT) 
    ->connect(xbox360.get_abs_port_in(Xbox360Driver::ABS_PORT_IN_RUMBLE_L));

  xbox360.get_abs_port_out(Xbox360Driver::XBOX360_AXIS_RT)
    ->connect(xbox360.get_abs_port_in(Xbox360Driver::ABS_PORT_IN_RUMBLE_R));

  xbox360.get_btn_port_out(Xbox360Driver::XBOX360_BTN_B)->connect(btn_change);

  xbox360.run();

  return 0;
}

/* EOF */
