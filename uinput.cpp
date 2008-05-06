/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdexcept>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include "xboxmsg.hpp"
#include "uinput.hpp"

uInput::uInput(GamepadType type, uInputCfg config_)
  : fd(-1), config(config_)
{
  // Open the input device
  const char* uinput_filename[] = { "/dev/input/uinput", "/dev/uinput", "/dev/misc/uinput" };
  const int uinput_filename_count = (sizeof(uinput_filename)/sizeof(char*));

  for (int i = 0; i < uinput_filename_count; ++i) 
    {
      if ((fd = open(uinput_filename[i], O_WRONLY | O_NDELAY)) >= 0)
        {
          break;
        }
      else
        {
          std::cout << "Error: " << uinput_filename[i] << ": " << strerror(errno) << std::endl;
        }
    }

  if (fd < 0)
    {
      std::cout << "Error: No stuitable uinput device found" << std::endl;
      std::cout << "" << std::endl;
      std::cout << "Troubleshooting:" << std::endl;
      std::cout << "  * make sure uinput kernel module is loaded " << std::endl;
      std::cout << "  * make sure joydev kernel module is loaded " << std::endl;
      std::cout << "  * make sure you have permissions to access the uinput device" << std::endl;
      std::cout << "  * start the driver with ./xboxdrv -v --no-uinput to see if the driver itself works" << std::endl;
      std::cout << "" << std::endl;
      exit(EXIT_FAILURE);
    }
  else
    {
      if (type == GAMEPAD_XBOX360 || type == GAMEPAD_XBOX || type == GAMEPAD_XBOX360_WIRELESS)
        {
          setup_xbox360_gamepad(type);
        }
      else if (type == GAMEPAD_XBOX360_GUITAR) 
        {
          setup_xbox360_guitar();
        }
      else
        {
          std::cout << "Unhandled type: " << type << std::endl;
          exit(EXIT_FAILURE);
        }

      if (ioctl(fd, UI_DEV_CREATE))
        {
          throw std::runtime_error("Unable to create UINPUT device.");
        }
    }
}

void
uInput::setup_xbox360_gamepad(GamepadType type)
{
  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ioctl(fd, UI_SET_EVBIT, EV_KEY);

  ioctl(fd, UI_SET_ABSBIT, ABS_X);
  ioctl(fd, UI_SET_ABSBIT, ABS_Y);

  if (!config.dpad_only)
    {  
      ioctl(fd, UI_SET_ABSBIT, ABS_RX);
      ioctl(fd, UI_SET_ABSBIT, ABS_RY);
    }

  if (config.trigger_as_button)
    {
      ioctl(fd, UI_SET_KEYBIT, BTN_TL2);
      ioctl(fd, UI_SET_KEYBIT, BTN_TR2);
    }
  else if (config.trigger_as_zaxis)
    {
      ioctl(fd, UI_SET_ABSBIT, ABS_Z);
    }
  else
    {
      ioctl(fd, UI_SET_ABSBIT, ABS_GAS);
      ioctl(fd, UI_SET_ABSBIT, ABS_BRAKE);
    }

  if (!config.dpad_only)
    {
      if (!config.dpad_as_button)
        {
          ioctl(fd, UI_SET_ABSBIT, ABS_HAT0X);
          ioctl(fd, UI_SET_ABSBIT, ABS_HAT0Y);
        }
      else
        {
          ioctl(fd, UI_SET_KEYBIT, BTN_BASE);
          ioctl(fd, UI_SET_KEYBIT, BTN_BASE2);
          ioctl(fd, UI_SET_KEYBIT, BTN_BASE3);
          ioctl(fd, UI_SET_KEYBIT, BTN_BASE4);
        }
    }

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
  strncpy(uinp.name, "Xbox Gamepad (userspace driver)", UINPUT_MAX_NAME_SIZE);
  uinp.id.version = 0;
  uinp.id.bustype = BUS_USB;
  uinp.id.vendor  = 0x045e; // FIXME: this shouldn't be hardcoded
  uinp.id.product = 0x028e;

  if (config.dpad_only)
    {
      uinp.absmin[ABS_X] = -1;
      uinp.absmax[ABS_X] =  1;
      
      uinp.absmin[ABS_Y] = -1;
      uinp.absmax[ABS_Y] =  1;
    }
  else
    {
      uinp.absmin[ABS_X] = -32768;
      uinp.absmax[ABS_X] =  32767;

      uinp.absmin[ABS_Y] = -32768;
      uinp.absmax[ABS_Y] =  32767;
    
      uinp.absmin[ABS_RX] = -32768;
      uinp.absmax[ABS_RX] =  32767;
      
      uinp.absmin[ABS_RY] = -32768;
      uinp.absmax[ABS_RY] =  32767;
    }

  if (config.trigger_as_zaxis)
    {
      uinp.absmin[ABS_Z] = -255;
      uinp.absmax[ABS_Z] =  255;         
    }
  else if (!config.trigger_as_button)
    {
      uinp.absmin[ABS_GAS] = 0;
      uinp.absmax[ABS_GAS] = 255;

      uinp.absmin[ABS_BRAKE] = 0;
      uinp.absmax[ABS_BRAKE] = 255;
    }
      
  if (!config.dpad_as_button && !config.dpad_only)
    {
      uinp.absmin[ABS_HAT0X] = -1;
      uinp.absmax[ABS_HAT0X] =  1;

      uinp.absmin[ABS_HAT0Y] = -1;
      uinp.absmax[ABS_HAT0Y] =  1;
    }

  write(fd, &uinp, sizeof(uinp));
}

void
uInput::setup_xbox360_guitar()
{
  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
        
  // Whammy and Tilt
  ioctl(fd, UI_SET_ABSBIT, ABS_X);
  ioctl(fd, UI_SET_ABSBIT, ABS_Y);

  // Dpad
  ioctl(fd, UI_SET_KEYBIT, BTN_BASE);
  ioctl(fd, UI_SET_KEYBIT, BTN_BASE2);
  ioctl(fd, UI_SET_KEYBIT, BTN_BASE3);
  ioctl(fd, UI_SET_KEYBIT, BTN_BASE4);

  // Base
  ioctl(fd, UI_SET_KEYBIT, BTN_START);
  ioctl(fd, UI_SET_KEYBIT, BTN_SELECT);
  ioctl(fd, UI_SET_KEYBIT, BTN_MODE);

  // Fret button
  ioctl(fd, UI_SET_KEYBIT, BTN_1);
  ioctl(fd, UI_SET_KEYBIT, BTN_2);
  ioctl(fd, UI_SET_KEYBIT, BTN_3);
  ioctl(fd, UI_SET_KEYBIT, BTN_4);
  ioctl(fd, UI_SET_KEYBIT, BTN_5);

  struct uinput_user_dev uinp;
  memset(&uinp,0,sizeof(uinp));
  strncpy(uinp.name, "Xbox360 Guitar (userspace driver)", UINPUT_MAX_NAME_SIZE);
  uinp.id.version = 0;
  uinp.id.bustype = BUS_USB;
  uinp.id.vendor  = 0x045e; // FIXME: Shouldn't be hardcoded
  uinp.id.product = 0x028e;

  uinp.absmin[ABS_X] = -32768;
  uinp.absmax[ABS_X] =  32767;

  uinp.absmin[ABS_Y] = -32768;
  uinp.absmax[ABS_Y] =  32767;

  
  write(fd, &uinp, sizeof(uinp));  
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
uInput::send(XboxGenericMsg& msg)
{
  switch(msg.type)
    {
      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        send(msg.xbox);
        break;

      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        send(msg.xbox360);
        break;

      case GAMEPAD_XBOX360_GUITAR:
        send(msg.guitar);
        break;
        
      default:
        std::cout << "XboxGenericMsg type: " << msg.type << std::endl;
        assert(!"uInput: Unknown XboxGenericMsg type");
    }
}

void
uInput::send(Xbox360Msg& msg)
{
  send_button(BTN_THUMBL,  msg.thumb_l);
  send_button(BTN_THUMBR,  msg.thumb_r);

  send_button(BTN_TL,  msg.lb);
  send_button(BTN_TR,  msg.rb);

  send_button(BTN_START,  msg.start);
  send_button(BTN_MODE,   msg.guide);
  send_button(BTN_SELECT, msg.back);

  send_button(BTN_A, msg.a);
  send_button(BTN_B, msg.b);
  send_button(BTN_X, msg.x);
  send_button(BTN_Y, msg.y);

  send_axis(ABS_X, msg.x1);
  send_axis(ABS_Y, -msg.y1);

  send_axis(ABS_RX, msg.x2);
  send_axis(ABS_RY, -msg.y2);

  if (config.trigger_as_zaxis)
    {
      send_axis(ABS_Z, (int(msg.rt) - int(msg.lt)));
    }
  else if (config.trigger_as_button)
    {
      send_button(BTN_TL2, msg.lt);
      send_button(BTN_TR2, msg.rt);    
    }
  else
    {
      send_axis(ABS_BRAKE, msg.lt);
      send_axis(ABS_GAS,   msg.rt);
    }
  
  if (config.dpad_as_button && !config.dpad_only)
    {
      send_button(BTN_BASE,  msg.dpad_up);
      send_button(BTN_BASE2, msg.dpad_down);
      send_button(BTN_BASE3, msg.dpad_left);
      send_button(BTN_BASE4, msg.dpad_right);
    }
  else
    {
      uint16_t dpad_x = ABS_HAT0X;
      uint16_t dpad_y = ABS_HAT0Y;
      
      if (config.dpad_only)
        {
          dpad_x = ABS_X;
          dpad_y = ABS_Y;
        }

      if (msg.dpad_up)
        {
          send_axis(dpad_y, -1);
        }
      else if (msg.dpad_down)
        {
          send_axis(dpad_y, 1);
        }
      else
        {
          send_axis(dpad_y, 0);
        }

      if (msg.dpad_left)
        {
          send_axis(dpad_x, -1);
        }
      else if (msg.dpad_right)
        {
          send_axis(dpad_x, 1);
        }
      else
        {
          send_axis(dpad_x, 0);
        }
    }
}

void
uInput::send(XboxMsg& msg)
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

  if (config.trigger_as_zaxis)
    {
      send_axis(ABS_Z, (int(msg.rt) - int(msg.lt)));
    }
  else if (config.trigger_as_button)
    {
      send_button(BTN_TL2, msg.lt);
      send_button(BTN_TR2, msg.rt);
    }
  else
    {
      send_axis(ABS_BRAKE, msg.lt);
      send_axis(ABS_GAS,   msg.rt);
    }

  if (config.dpad_as_button)
    {
      send_button(BTN_BASE,  msg.dpad_up);
      send_button(BTN_BASE2, msg.dpad_down);
      send_button(BTN_BASE3, msg.dpad_left);
      send_button(BTN_BASE4, msg.dpad_right);
    }
  else
    {
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
}

void
uInput::send(Xbox360GuitarMsg& msg)
{
  send_button(BTN_BASE,  msg.dpad_up);
  send_button(BTN_BASE2, msg.dpad_down);
  send_button(BTN_BASE3, msg.dpad_left);
  send_button(BTN_BASE4, msg.dpad_right);

  send_button(BTN_START,  msg.start);
  send_button(BTN_MODE,   msg.guide);
  send_button(BTN_SELECT, msg.back);

  send_button(BTN_1, msg.green);
  send_button(BTN_2, msg.red);
  send_button(BTN_3, msg.yellow);
  send_button(BTN_4, msg.blue);
  send_button(BTN_5, msg.orange);

  send_axis(ABS_X, msg.whammy);
  send_axis(ABS_Y, msg.tilt);
}

/* EOF */
