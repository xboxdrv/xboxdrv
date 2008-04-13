/* 
**  XBox360 USB Gamepad Userspace Driver
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

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include "uinput.hpp"

uInput::uInput(GamepadType type)
{
  // Open the input device
  fd = open("/dev/input/uinput", O_WRONLY | O_NDELAY);
  if (!fd)
    {
      std::cout << "Unable to open /dev/input/uinput" << std::endl;
    }
  else
    {
      ioctl(fd, UI_SET_EVBIT, EV_ABS);
        
      ioctl(fd, UI_SET_ABSBIT, ABS_X);
      ioctl(fd, UI_SET_ABSBIT, ABS_Y);

      ioctl(fd, UI_SET_ABSBIT, ABS_RX);
      ioctl(fd, UI_SET_ABSBIT, ABS_RY);

      ioctl(fd, UI_SET_ABSBIT, ABS_GAS);
      ioctl(fd, UI_SET_ABSBIT, ABS_BRAKE);

      ioctl(fd, UI_SET_ABSBIT, ABS_HAT0X);
      ioctl(fd, UI_SET_ABSBIT, ABS_HAT0Y);

      ioctl(fd, UI_SET_EVBIT,  EV_KEY);

      ioctl(fd, UI_SET_KEYBIT, BTN_START);
      ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
        
      if (type == GAMEPAD_XBOX360 || type == GAMEPAD_XBOX360_WIRELESS)
        ioctl(fd, UI_SET_KEYBIT, BTN_MODE);

      ioctl(fd, UI_SET_KEYBIT, BTN_A);
      ioctl(fd, UI_SET_KEYBIT, BTN_B);
      ioctl(fd, UI_SET_KEYBIT, BTN_X);
      ioctl(fd, UI_SET_KEYBIT, BTN_Y);

      ioctl(fd, UI_SET_KEYBIT, BTN_TL);
      ioctl(fd, UI_SET_KEYBIT, BTN_TR);

      ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL);
      ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR);

      struct uinput_user_dev uinp;
      memset(&uinp,0,sizeof(uinp));
      strncpy(uinp.name, "XBox Gamepad (userspace driver)", UINPUT_MAX_NAME_SIZE);
      uinp.id.version = 0;
      uinp.id.bustype = BUS_USB;
      uinp.id.vendor  = 0x045e;
      uinp.id.product = 0x028e;

      uinp.absmin[ABS_X] = -32768;
      uinp.absmax[ABS_X] =  32767;

      uinp.absmin[ABS_Y] = -32768;
      uinp.absmax[ABS_Y] =  32767;

      uinp.absmin[ABS_RX] = -32768;
      uinp.absmax[ABS_RX] =  32767;

      uinp.absmin[ABS_RY] = -32768;
      uinp.absmax[ABS_RY] =  32767;

      uinp.absmin[ABS_GAS] = 0;
      uinp.absmax[ABS_GAS] = 255;
    
      uinp.absmin[ABS_BRAKE] = 0;
      uinp.absmax[ABS_BRAKE] = 255;

      uinp.absmin[ABS_HAT0X] = -1;
      uinp.absmax[ABS_HAT0X] =  1;

      uinp.absmin[ABS_HAT0Y] = -1;
      uinp.absmax[ABS_HAT0Y] =  1;

      write(fd, &uinp, sizeof(uinp));

      if (ioctl(fd, UI_DEV_CREATE))
        {
          std::cout << "Unable to create UINPUT device." << std::endl;
        }
    }
}

uInput::~uInput()
{
  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
}

void
uInput::send_button(uint16_t code, int32_t value)
{
  struct input_event ev;      
  memset(&ev, 0, sizeof(ev));

  gettimeofday(&ev.time, NULL);
  ev.type  = EV_KEY;
  ev.code  = code;
  ev.value = (value>0) ? 1 : 0;

 write(fd, &ev, sizeof(ev));
}

void
uInput::send_axis(uint16_t code, int32_t value)
{
  struct input_event ev;      
  memset(&ev, 0, sizeof(ev));

  gettimeofday(&ev.time, NULL);
  ev.type  = EV_ABS;
  ev.code  = code;
  ev.value = value;

 write(fd, &ev, sizeof(ev));  
}

void
uInput::send(XBox360Msg& msg)
{
  send_button(BTN_THUMBL,  msg.thumb_l);
  send_button(BTN_THUMBR,  msg.thumb_r);

  send_button(BTN_TL,  msg.lb);
  send_button(BTN_TR,  msg.rb);

  send_button(BTN_START,  msg.start);
  send_button(BTN_MODE,   msg.mode);
  send_button(BTN_SELECT, msg.select);

  send_button(BTN_A, msg.a);
  send_button(BTN_B, msg.b);
  send_button(BTN_X, msg.x);
  send_button(BTN_Y, msg.y);

  send_axis(ABS_X, msg.x1);
  send_axis(ABS_Y, -msg.y1);

  send_axis(ABS_RX, msg.x2);
  send_axis(ABS_RY, -msg.y2);

  send_axis(ABS_BRAKE, msg.lt);
  send_axis(ABS_GAS,   msg.rt);

  if (msg.dpad_up)
    {
      send_axis(ABS_HAT0Y, -1);
    }
  else if (msg.dpad_down)
    {
      send_axis(ABS_HAT0Y, 1);
    }
  else
    {
      send_axis(ABS_HAT0Y, 0);
    }

  if (msg.dpad_left)
    {
      send_axis(ABS_HAT0X, -1);
    }
  else if (msg.dpad_right)
    {
      send_axis(ABS_HAT0X, 1);
    }
  else
    {
      send_axis(ABS_HAT0X, 0);
    }
}

void
uInput::send(XBoxMsg& msg)
{
  send_button(BTN_THUMBL,  msg.thumb_l);
  send_button(BTN_THUMBR,  msg.thumb_r);

  send_button(BTN_TL,  msg.white);
  send_button(BTN_TR,  msg.black);

  send_button(BTN_START,  msg.start);
  send_button(BTN_SELECT, msg.back);

  send_button(BTN_A, msg.a);
  send_button(BTN_B, msg.b);
  send_button(BTN_X, msg.x);
  send_button(BTN_Y, msg.y);

  send_axis(ABS_X, msg.x1);
  send_axis(ABS_Y, msg.y1);

  send_axis(ABS_RX, msg.x2);
  send_axis(ABS_RY, msg.y2);

  send_axis(ABS_BRAKE, msg.lt);
  send_axis(ABS_GAS,   msg.rt);

  if (msg.dpad_up)
    {
      send_axis(ABS_HAT0Y, -1);
    }
  else if (msg.dpad_down)
    {
      send_axis(ABS_HAT0Y, 1);
    }
  else
    {
      send_axis(ABS_HAT0Y, 0);
    }

  if (msg.dpad_left)
    {
      send_axis(ABS_HAT0X, -1);
    }
  else if (msg.dpad_right)
    {
      send_axis(ABS_HAT0X, 1);
    }
  else
    {
      send_axis(ABS_HAT0X, 0);
    }
}

/* EOF */
