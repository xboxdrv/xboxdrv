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
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include "xbox360_driver.hpp"

struct usb_device* find_usb_device_by_path(const std::string& busid, const std::string& devid) { return 0; }
struct usb_device* find_usb_device_by_ids(uint16_t idVendor, uint16_t idProduct) { return 0; }

XPadDevice xbox360_devices[] = {
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller"  },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
};

const int xbox360_devices_count = sizeof(xbox360_devices)/sizeof(XPadDevice);

void
Xbox360Driver::init()
{
  dev    = 0;
  handle = 0;

  for(int i = 0; i < XBOX360_BTN_LENGTH; ++i)
    btn_port_out.push_back(new BtnPortOut((boost::format("Xbox360Driver Button %d") % i).str()));

  // This should really be abs ports so that one can select different
  // rumble strength an LED status
  btn_port_in.push_back(new BtnPortIn("Xbox360Driver LED",    boost::function<void(BtnPortOut*)>()));
  btn_port_in.push_back(new BtnPortIn("Xbox360Driver Rumble", boost::function<void(BtnPortOut*)>()));
}

Xbox360Driver::Xbox360Driver(const std::string& busid, const std::string& devid)
{
  init();

  dev = find_usb_device_by_path(busid, devid);
  open_dev();
}

Xbox360Driver::Xbox360Driver(int id)
{
  init();

  for(int i = 0; i < xbox360_devices_count && !dev; ++i)
    {
      if (xbox360_devices[i].type == GAMEPAD_XBOX360 ||
          xbox360_devices[i].type == GAMEPAD_XBOX360_GUITAR)
        {
          dev = find_usb_device_by_ids(xbox360_devices[i].idVendor,
                                       xbox360_devices[i].idProduct);
        }
    }

  open_dev();
}

Xbox360Driver::~Xbox360Driver()
{
  close_dev();
}

void
Xbox360Driver::open_dev()
{
  if (!dev)
    {
      throw std::runtime_error("Xbox360Driver: Couldn't find suitable USB device");
    }
  else
    {
      handle = usb_open(dev);
    }
}

void
Xbox360Driver::close_dev()
{
  usb_close(handle);
}

void
Xbox360Driver::run()
{ // Run this in a seperate Thread
  bool quit = false;
  uint8_t old_data[20];
  memset(old_data, 0, 20);
  while(!quit)
    {
      uint8_t data[20];

      int ret = usb_interrupt_read(handle, 1 /*EndPoint*/, (char*)data, 20, 0 /*Timeout*/);

      if (ret < 0)
        { // Error
          std::cout << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
          std::cout << "Shutting down" << std::endl;
          quit = true;
        }
      else if (ret == 0) // ignore
        {
          // happen with the Xbox360 every now and then, just
          // ignore, seems harmless
        }
      else if (ret == 3) // ignore
        {
          // This data gets send when the controller is accessed the
          // first time after being connected to the USB bus, no idea
          // what it means, it seems to be identical for different
          // controllers, we just ignore it
          //
          // len: 3 Data: 0x01 0x03 0x0e
          // len: 3 Data: 0x02 0x03 0x00
          // len: 3 Data: 0x03 0x03 0x03
          // len: 3 Data: 0x08 0x03 0x00
          // len: 3 Data: 0x01 0x03 0x00
        }
      else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
        {
          if (memcmp(data, old_data, 20) == 0)
            {
              // Ignore the data, since nothing has changed
            }                
          else
            {
              memcpy(old_data, data, 20);
              Xbox360Msg& msg = (Xbox360Msg&)data;
              update(msg);
            }                  
        }
      else
        {
          std::cout << "Unknown data: bytes: " << ret << " Data: ";
          for(int j = 0; j < ret; ++j)
            std::cout << boost::format("0x%02x ") % int(data[j]);
          std::cout << std::endl;
        } 
    }
}

void
Xbox360Driver::update(const Xbox360Msg& msg)
{
  btn_port_out[XBOX360_DPAD_UP]   ->set_state(msg.dpad_up);
  btn_port_out[XBOX360_DPAD_DOWN] ->set_state(msg.dpad_down);
  btn_port_out[XBOX360_DPAD_LEFT] ->set_state(msg.dpad_left);
  btn_port_out[XBOX360_DPAD_RIGHT]->set_state(msg.dpad_right);

  btn_port_out[XBOX360_BTN_A]->set_state(msg.a);
  btn_port_out[XBOX360_BTN_B]->set_state(msg.b);
  btn_port_out[XBOX360_BTN_X]->set_state(msg.x);
  btn_port_out[XBOX360_BTN_Y]->set_state(msg.y);

  btn_port_out[XBOX360_BTN_LB]->set_state(msg.lb);
  btn_port_out[XBOX360_BTN_RB]->set_state(msg.rb);

  btn_port_out[XBOX360_BTN_START]->set_state(msg.start);
  btn_port_out[XBOX360_BTN_BACK]->set_state(msg.back);
  btn_port_out[XBOX360_BTN_MODE]->set_state(msg.mode);
}

void
Xbox360Driver::set_led(uint8_t led_status)
{
  char ledcmd[] = { 1, 3, led_status }; 
  usb_interrupt_write(handle, 2, ledcmd, 3, 0);
}

void
Xbox360Driver::set_rumble(uint8_t big, uint8_t small)
{
  char rumblecmd[] = { 0x00, 0x06, 0x00, small/*right*/, 0x00, big/*left*/};
  usb_interrupt_write(handle, 2, rumblecmd, 6, 0);    
}

/* EOF */
