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

#include <usb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "xbox360_driver.hpp"
#include "uinput_driver.hpp"
#include "abs_to_rel.hpp"
#include "toggle_button.hpp"
#include "autofire_button.hpp"
#include "join_axis.hpp"
#include "btn_to_abs.hpp"
#include "throttle.hpp"
#include "evdev_driver.hpp"
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

  try 
    {
      //EvdevDriver* evdev = new EvdevDriver("/dev/input/event10");

      UInputDriver* uinput = new UInputDriver("UInputMouseEmulation");

      uinput->add_abs(ABS_X, -32767, 32767);
      uinput->add_abs(ABS_Y, -32767, 32767);
      uinput->add_abs(ABS_THROTTLE, 0, 32767);

      uinput->finish();

      std::vector<Control*> controls;

      Xbox360Driver*  xbox360       = new Xbox360Driver(0);
      Throttle*       throttle      = new Throttle();

      controls.push_back(xbox360);
      controls.push_back(uinput);
      controls.push_back(throttle);

      // ----------------------------

      connect_abs(xbox360, Xbox360Driver::XBOX360_AXIS_X1, uinput, 0);
      connect_abs(xbox360, Xbox360Driver::XBOX360_AXIS_Y1, uinput, 1);

      connect_abs(xbox360,  Xbox360Driver::XBOX360_AXIS_Y2, throttle, 0);
      connect_abs(throttle, 0, uinput,   2);
  
      // ----------------------------

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
    }
  catch(std::runtime_error& err)
    {
      std::cerr << "Error: " << err.what() << std::endl;
    }
  
  return 0;
}

/* EOF */
