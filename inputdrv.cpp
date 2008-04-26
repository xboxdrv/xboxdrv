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
#include "uinput_driver.hpp"
#include "abs_to_rel.hpp"
#include "toggle_button.hpp"
#include "autofire_button.hpp"
#include "join_axis.hpp"
#include "btn_to_abs.hpp"
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

  UInputDriver* uinput = new UInputDriver("UInputMouseEmulation");

  uinput->add_rel(REL_X);
  uinput->add_rel(REL_Y);
  uinput->add_rel(REL_HWHEEL);
  uinput->add_rel(REL_WHEEL);

  uinput->add_btn(BTN_LEFT);
  uinput->add_btn(BTN_RIGHT);
  uinput->add_btn(BTN_MIDDLE);
  uinput->add_btn(BTN_Y);

  uinput->add_abs(ABS_X, -32767, 32767);
  uinput->add_abs(ABS_Y, -32767, 32767);
  uinput->add_abs(ABS_Z, -255, 255);

  uinput->finish();

  std::vector<Control*> controls;

  Xbox360Driver*  xbox360       = new Xbox360Driver(0);
  ToggleButton*   toggle        = new ToggleButton();
  AbsToRel*       abs_to_rel_x  = new AbsToRel();
  AbsToRel*       abs_to_rel_y  = new AbsToRel();
  AbsToRel*       abs_to_rel_x2 = new AbsToRel();
  AbsToRel*       abs_to_rel_y2 = new AbsToRel();
  AutofireButton* autofire      = new AutofireButton(50);
  JoinAxis*       join_axis     = new JoinAxis();
  BtnToAbs*       btn_to_abs_x  = new BtnToAbs();
  BtnToAbs*       btn_to_abs_y  = new BtnToAbs();

  controls.push_back(xbox360);
  controls.push_back(toggle);
  controls.push_back(abs_to_rel_x);
  controls.push_back(abs_to_rel_y);
  controls.push_back(abs_to_rel_x2);
  controls.push_back(abs_to_rel_y2);
  controls.push_back(autofire);
  controls.push_back(join_axis);
  controls.push_back(btn_to_abs_x);
  controls.push_back(btn_to_abs_y);

  // ----------------------------

  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_X1) 
    ->connect(abs_to_rel_x->get_abs_port_in(0));
  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_Y1) 
    ->connect(abs_to_rel_y->get_abs_port_in(0));

  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_X2) 
    ->connect(abs_to_rel_x2->get_abs_port_in(0));
  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_Y2) 
    ->connect(abs_to_rel_y2->get_abs_port_in(0));

  abs_to_rel_x->get_rel_port_out(0)
    ->connect(uinput->get_rel_port_in(0));
  abs_to_rel_y->get_rel_port_out(0)
    ->connect(uinput->get_rel_port_in(1));
  abs_to_rel_x2->get_rel_port_out(0)
    ->connect(uinput->get_rel_port_in(2));
  abs_to_rel_y2->get_rel_port_out(0)
    ->connect(uinput->get_rel_port_in(3));

  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_BTN_A) 
    ->connect(uinput->get_btn_port_in(0));
  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_BTN_B) 
    ->connect(uinput->get_btn_port_in(1));
  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_BTN_X) 
    ->connect(uinput->get_btn_port_in(2));

  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_BTN_Y) 
    ->connect(autofire->get_btn_port_in(0));
  autofire->get_btn_port_out(0)
    ->connect(uinput->get_btn_port_in(3));
  
  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_LT)
    ->connect(join_axis->get_abs_port_in(0));
  xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_RT)
    ->connect(join_axis->get_abs_port_in(1));

  join_axis->get_abs_port_out(0)
    ->connect(uinput->get_abs_port_in(2));


  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_DPAD_LEFT)
    ->connect(btn_to_abs_x->get_btn_port_in(0));
  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_DPAD_RIGHT)
    ->connect(btn_to_abs_x->get_btn_port_in(1));
  btn_to_abs_x->get_abs_port_out(0)
    ->connect(uinput->get_abs_port_in(0));

  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_DPAD_UP)
    ->connect(btn_to_abs_y->get_btn_port_in(0));
  xbox360->get_btn_port_out(Xbox360Driver::XBOX360_DPAD_DOWN)
    ->connect(btn_to_abs_y->get_btn_port_in(1));  
  btn_to_abs_y->get_abs_port_out(0)
    ->connect(uinput->get_abs_port_in(1));

  // ----------------------------

  if (0)
    { // rumble stuff
      xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_LT) 
        ->connect(xbox360->get_abs_port_in(Xbox360Driver::ABS_PORT_IN_RUMBLE_L));

      xbox360->get_abs_port_out(Xbox360Driver::XBOX360_AXIS_RT)
        ->connect(xbox360->get_abs_port_in(Xbox360Driver::ABS_PORT_IN_RUMBLE_R));

      xbox360->get_btn_port_out(Xbox360Driver::XBOX360_BTN_Y)->connect(btn_change);
    }

  bool quit = false;
  while(!quit)
    {
      for(std::vector<Control*>::iterator i = controls.begin(); i != controls.end(); ++i)
        {
          (*i)->update(0.001f);
        }
      //std::cout << "." << std::flush;
      usleep(1000); // 0.001sec or 1msec
    }

  return 0;
}

/* EOF */
