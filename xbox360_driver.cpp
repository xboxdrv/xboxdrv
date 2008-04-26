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
#include <errno.h>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include "log.hpp"
#include "xbox360_usb_thread.hpp"
#include "xbox360_driver.hpp"

struct usb_device* find_usb_device_by_path(const std::string& busid, const std::string& devid) 
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      if (bus->dirname == busid)
        {
          for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
            {
              if (dev->filename == devid)
                {
                  return dev;
                }
            }
        }
    }
  return 0;
}

struct usb_device* find_usb_device_by_ids(int id, uint16_t idVendor, uint16_t idProduct) 
{
  struct usb_bus* busses = usb_get_busses();

  int id_count = 0;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          if (dev->descriptor.idVendor  == idVendor &&
              dev->descriptor.idProduct == idProduct)
            {
              if (id_count == id)
                {
                  return dev;
                }
              else
                {
                  id_count += 1;
                }
            }
        }
    }
  return 0; 
}

XPadDevice xbox360_devices[] = {
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller"  },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller"   },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
};

const int xbox360_devices_count = sizeof(xbox360_devices)/sizeof(XPadDevice);

void
Xbox360Driver::init()
{
  rumble_l = 0;
  rumble_r = 0;

  for(int i = 0; i < XBOX360_BTN_LENGTH; ++i)
    btn_port_out.push_back(new BtnPortOut((boost::format("Xbox360Driver Button %d") % i).str()));

  // This should really be abs ports so that one can select different
  // rumble strength an LED status
  btn_port_in.push_back(new BtnPortIn("Xbox360Driver LED",    
                                      boost::bind(&Xbox360Driver::on_led_btn, this, _1)));

  abs_port_in.push_back(new AbsPortIn("Xbox360Driver Rumble Left", 0, 255,
                                      boost::bind(&Xbox360Driver::on_rumble_left_abs, this, _1)));
  abs_port_in.push_back(new AbsPortIn("Xbox360Driver Rumble Right", 0, 255,
                                      boost::bind(&Xbox360Driver::on_rumble_right_abs, this, _1)));

  abs_port_out.push_back(new AbsPortOut("Xbox360Driver X1-Axis", -32768, 32767));
  abs_port_out.push_back(new AbsPortOut("Xbox360Driver Y1-Axis", -32768, 32767));
  abs_port_out.push_back(new AbsPortOut("Xbox360Driver X2-Axis", -32768, 32767));
  abs_port_out.push_back(new AbsPortOut("Xbox360Driver Y2-Axis", -32768, 32767));
  abs_port_out.push_back(new AbsPortOut("Xbox360Driver LT-Axis", 0, 255));
  abs_port_out.push_back(new AbsPortOut("Xbox360Driver RT-Axis", 0, 255));
}

Xbox360Driver::Xbox360Driver(const std::string& busid, const std::string& devid)
{
  init();

  struct usb_device* dev = find_usb_device_by_path(busid, devid);
  if (!dev)
    {
      throw std::runtime_error("Xbox360Driver: Couldn't find suitable USB device");
    }
  else
    {
      thread = new Xbox360UsbThread(dev);
      thread->start();
    }
}

Xbox360Driver::Xbox360Driver(int id)
{
  init();

  struct usb_device* dev = NULL;
  // FIXME: This loop can't work
  for(int i = 0; i < xbox360_devices_count && !dev; ++i)
    {
      if (xbox360_devices[i].type == GAMEPAD_XBOX360 ||
          xbox360_devices[i].type == GAMEPAD_XBOX360_GUITAR)
        {
          dev = find_usb_device_by_ids(id, 
                                       xbox360_devices[i].idVendor,
                                       xbox360_devices[i].idProduct);
        }
    }

  if (!dev)
    {
      throw std::runtime_error("Xbox360Driver: Couldn't find suitable USB device");
    }
  else
    {
      thread = new Xbox360UsbThread(dev);
      thread->start();
    }
}

Xbox360Driver::~Xbox360Driver()
{
  thread->stop();
  delete thread;
}

void
Xbox360Driver::update(float delta)
{
  while(thread->has_msg())
    {
      process_msg(thread->pop_msg());
    }
}

void
Xbox360Driver::process_msg(const Xbox360Msg& msg)
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

  btn_port_out[XBOX360_BTN_THUMB_L]->set_state(msg.thumb_l);
  btn_port_out[XBOX360_BTN_THUMB_R]->set_state(msg.thumb_r);

  btn_port_out[XBOX360_BTN_START]->set_state(msg.start);
  btn_port_out[XBOX360_BTN_BACK]->set_state(msg.back);
  btn_port_out[XBOX360_BTN_GUIDE]->set_state(msg.guide);

  abs_port_out[XBOX360_AXIS_X1]->set_state(msg.x1);
  abs_port_out[XBOX360_AXIS_Y1]->set_state(-msg.y1);

  abs_port_out[XBOX360_AXIS_X2]->set_state(msg.x2);
  abs_port_out[XBOX360_AXIS_Y2]->set_state(-msg.y2);

  abs_port_out[XBOX360_AXIS_LT]->set_state(msg.lt);
  abs_port_out[XBOX360_AXIS_RT]->set_state(msg.rt);
}

void
Xbox360Driver::on_led_btn(BtnPortOut* btn)
{
  if (btn->get_state())
    {
      thread->set_led(10);
    }
  else
    {
      thread->set_led(0);
    }
}

void
Xbox360Driver::on_rumble_left_abs(AbsPortOut* abs)
{
  rumble_l = 255 * (abs->get_state() - abs->min_value) / (abs->max_value - abs->min_value);
  thread->set_rumble(rumble_l, rumble_r);
}

void
Xbox360Driver::on_rumble_right_abs(AbsPortOut* abs)
{
  rumble_r = 255 * (abs->get_state() - abs->min_value) / (abs->max_value - abs->min_value);
  thread->set_rumble(rumble_l, rumble_r);
}

/* EOF */
