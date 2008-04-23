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
#include "xbox360_driver.hpp"

XPadDevice xbox360_devices[] = {
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller"  },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
}

const int xbox360_devices_count = sizeof(xpad_devices)/sizeof(XPadDevice);

void
XBox360Driver::init()
{
  dev    = 0;
  handle = 0;
  for(int i = 0; i < XBOX360_BTN_LENGTH; ++i)
    buttons.push_back(new Button(NULL));
}

Xbox360Driver::Xbox360Driver(const std::string& busid, const std::string& devid)
{
  init();

  dev = find_usb_device_by_path(opts.busid, opts.devid);
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
      throw std::runtime_error("XBox360Driver: Couldn't find device " + busid + " " + devid);
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

Button*
XBox360Driver::create_button(const std::string& name)
{
  if      (name == "X")
    {
      return buttons[XBOX360_BTN_X];
    }
  else if (name == "Y")
    {
      return buttons[XBOX360_BTN_Y];
    }
  else if (name == "A")
    {
      return buttons[XBOX360_BTN_A];
    }
  else if (name == "B")
    {
      return buttons[XBOX360_BTN_B];
    }
  else if (name == "LB")
    {
      return buttons[XBOX360_BTN_LB];
    }
  else if (name == "RB")
    {
      return buttons[XBOX360_BTN_RB];
    }
  else if (name == "Start")
    {
      return buttons[XBOX360_BTN_START];
    }
  else if (name == "Back")
    {
      return buttons[XBOX360_BTN_BACK];
    }
  else if (name == "Mode" || name == "XBox360")
    {
      return buttons[XBOX360_BTN_MODE];
    }
  else
    {
      throw std::runtime_error("Unknown button: " + name);
    }
}

void
XBox360Driver::run()
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
          // happen with the XBox360 every now and then, just
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
              XBox360Msg& msg = (XBox360Msg&)data;
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
XBox360Driver::update(const XBox360Msg& msg)
{
  buttons[XBOX360_DPAD_UP]   ->set_state(msg.dpad_up);
  buttons[XBOX360_DPAD_DOWN] ->set_state(msg.dpad_down);
  buttons[XBOX360_DPAD_LEFT] ->set_state(msg.dpad_left);
  buttons[XBOX360_DPAD_RIGHT]->set_state(msg.dpad_right);

  buttons[XBOX360_BTN_A]->set_state(msg.a);
  buttons[XBOX360_BTN_B]->set_state(msg.b);
  buttons[XBOX360_BTN_X]->set_state(msg.x);
  buttons[XBOX360_BTN_Y]->set_state(msg.y);

  buttons[XBOX360_BTN_LB]->set_state(msg.lb);
  buttons[XBOX360_BTN_RB]->set_state(msg.rb);

  buttons[XBOX360_BTN_START]->set_state(msg.start);
  buttons[XBOX360_BTN_BACK]->set_state(msg.back);
  buttons[XBOX360_BTN_MODE]->set_state(msg.mode);
}

void
XBox360Driver::set_led(int led_status)
{
  char ledcmd[] = { 1, 3, led_status }; 
  usb_interrupt_write(handle, 2, ledcmd, 3, 0);
}

void
XBox360Driver::set_rumble(uint8_t big, uint8_t small)
{
  char rumblecmd[] = { 0x00, 0x06, 0x00, small/*right*/, 0x00, big/*left*/};
  usb_interrupt_write(handle, 2, rumblecmd, 6, 0);    
}

/* EOF */
