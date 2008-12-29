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
#include <linux/input.h>
#include <linux/uinput.h>

#include "xboxmsg.hpp"
#include "uinput.hpp"

uInputCfg::uInputCfg() 
{
  trigger_as_button = false;
  dpad_as_button    = false;
  trigger_as_zaxis  = false;
  dpad_only         = false;
    
  // Button Mapping
  btn_map[XBOX_BTN_START] = BTN_START;
  btn_map[XBOX_BTN_GUIDE] = BTN_MODE;
  btn_map[XBOX_BTN_BACK]  = BTN_SELECT;

  btn_map[XBOX_BTN_A] = BTN_A;
  btn_map[XBOX_BTN_B] = BTN_B;
  btn_map[XBOX_BTN_X] = BTN_X;
  btn_map[XBOX_BTN_Y] = BTN_Y;

  btn_map[XBOX_BTN_WHITE] = BTN_TL;
  btn_map[XBOX_BTN_BLACK] = BTN_TR;

  btn_map[XBOX_BTN_LB] = BTN_TL;
  btn_map[XBOX_BTN_RB] = BTN_TR;

  btn_map[XBOX_BTN_LT] = BTN_TL2;
  btn_map[XBOX_BTN_RT] = BTN_TR2;

  btn_map[XBOX_BTN_THUMB_L] = BTN_THUMBL;
  btn_map[XBOX_BTN_THUMB_R] = BTN_THUMBR;
  
  btn_map[XBOX_DPAD_UP]    = BTN_BASE;
  btn_map[XBOX_DPAD_DOWN]  = BTN_BASE2;
  btn_map[XBOX_DPAD_LEFT]  = BTN_BASE3;
  btn_map[XBOX_DPAD_RIGHT] = BTN_BASE4;

  // Axis Mapping
  axis_map[XBOX_AXIS_X1] = ABS_X; 
  axis_map[XBOX_AXIS_Y1] = ABS_Y; 
  axis_map[XBOX_AXIS_X2] = ABS_RX; 
  axis_map[XBOX_AXIS_Y2] = ABS_RY; 
  axis_map[XBOX_AXIS_LT] = ABS_GAS;
  axis_map[XBOX_AXIS_RT] = ABS_BRAKE; 
  axis_map[XBOX_AXIS_TRIGGER] = ABS_Z; 
  axis_map[XBOX_AXIS_DPAD_X] = ABS_HAT0X;
  axis_map[XBOX_AXIS_DPAD_Y] = ABS_HAT0Y;
}

uInput::uInput(GamepadType type, uInputCfg config_)
  : fd(-1), cfg(config_)
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

  ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_X1]);
  ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_Y1]);

  if (!cfg.dpad_only)
    {  
      ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_X2]);
      ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_Y2]);
    }

  if (cfg.trigger_as_button)
    {
      ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_LT]);
      ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_RT]);
    }
  else if (cfg.trigger_as_zaxis)
    {
      ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_TRIGGER]);
    }
  else
    {
      ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_LT]);
      ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_RT]);
    }

  if (!cfg.dpad_only)
    {
      if (!cfg.dpad_as_button)
        {
          ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_DPAD_X]);
          ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_DPAD_Y]);
        }
      else
        {
          ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_UP]);
          ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_DOWN]);
          ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_LEFT]);
          ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_RIGHT]);
        }
    }

  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_START]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_BACK]);
        
  if (type == GAMEPAD_XBOX360 || type == GAMEPAD_XBOX360_WIRELESS)
    ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_GUIDE]);

  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_A]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_B]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_X]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_Y]);

  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_LB]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_RB]);

  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_THUMB_L]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_THUMB_R]);

  struct uinput_user_dev uinp;
  memset(&uinp,0,sizeof(uinp));
  
  strncpy(uinp.name, "Xbox Gamepad (userspace driver)", UINPUT_MAX_NAME_SIZE);

  uinp.id.version = 0;
  uinp.id.bustype = BUS_USB;
  uinp.id.vendor  = 0x045e; // FIXME: this shouldn't be hardcoded
  uinp.id.product = 0x028e;

  if (cfg.dpad_only)
    {
      // FIXME: Adjust properly to work with --ui-axismap
      uinp.absmin[cfg.axis_map[XBOX_AXIS_X1]] = -1;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_X1]] =  1;
      
      uinp.absmin[cfg.axis_map[XBOX_AXIS_Y1]] = -1;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_Y1]] =  1;
    }
  else
    {
      uinp.absmin[cfg.axis_map[XBOX_AXIS_X1]] = -32768;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_X1]] =  32767;

      uinp.absmin[cfg.axis_map[XBOX_AXIS_Y1]] = -32768;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_Y1]] =  32767;
    
      uinp.absmin[cfg.axis_map[XBOX_AXIS_X2]] = -32768;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_X2]] =  32767;
      
      uinp.absmin[cfg.axis_map[XBOX_AXIS_Y2]] = -32768;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_Y2]] =  32767;
    }

  if (cfg.trigger_as_zaxis)
    {
      uinp.absmin[cfg.axis_map[XBOX_AXIS_TRIGGER]] = -255;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_TRIGGER]] =  255;         
    }
  else if (!cfg.trigger_as_button)
    {
      uinp.absmin[cfg.axis_map[XBOX_AXIS_LT]] = 0;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_LT]] = 255;

      uinp.absmin[cfg.axis_map[XBOX_AXIS_RT]] = 0;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_RT]] = 255;
    }
      
  if (!cfg.dpad_as_button && !cfg.dpad_only)
    {
      uinp.absmin[cfg.axis_map[XBOX_AXIS_DPAD_X]] = -1;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_DPAD_X]] =  1;

      uinp.absmin[cfg.axis_map[XBOX_AXIS_DPAD_Y]] = -1;
      uinp.absmax[cfg.axis_map[XBOX_AXIS_DPAD_Y]] =  1;
    }

  if (write(fd, &uinp, sizeof(uinp)) < 0)
    throw std::runtime_error(strerror(errno));
}

void
uInput::setup_xbox360_guitar()
{
  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
        
  // Whammy and Tilt
  ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_X1]);
  ioctl(fd, UI_SET_ABSBIT, cfg.axis_map[XBOX_AXIS_Y1]);

  // Dpad
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_UP]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_DOWN]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_LEFT]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_DPAD_RIGHT]);

  // Base
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_START]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_BACK]);
  ioctl(fd, UI_SET_KEYBIT, cfg.btn_map[XBOX_BTN_GUIDE]);

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

  uinp.absmin[cfg.btn_map[XBOX_BTN_X]] = -32768;
  uinp.absmax[cfg.btn_map[XBOX_BTN_X]] =  32767;

  uinp.absmin[cfg.btn_map[XBOX_BTN_Y]] = -32768;
  uinp.absmax[cfg.btn_map[XBOX_BTN_Y]] =  32767;

  
  if (write(fd, &uinp, sizeof(uinp)) < 0)
    throw std::runtime_error(strerror(errno));
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

  if (write(fd, &ev, sizeof(ev)) < 0)
    throw std::runtime_error(strerror(errno));
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

  if (write(fd, &ev, sizeof(ev)) < 0)
    throw std::runtime_error(strerror(errno));
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

  send_button(cfg.btn_map[XBOX_BTN_START], msg.start);
  send_button(cfg.btn_map[XBOX_BTN_GUIDE], msg.guide);
  send_button(cfg.btn_map[XBOX_BTN_BACK],  msg.back);

  send_button(cfg.btn_map[XBOX_BTN_A], msg.a);
  send_button(cfg.btn_map[XBOX_BTN_B], msg.b);
  send_button(cfg.btn_map[XBOX_BTN_X], msg.x);
  send_button(cfg.btn_map[XBOX_BTN_Y], msg.y);

  send_axis(cfg.axis_map[XBOX_AXIS_X1],  msg.x1);
  send_axis(cfg.axis_map[XBOX_AXIS_Y1], -msg.y1);

  send_axis(cfg.axis_map[XBOX_AXIS_X2],  msg.x2);
  send_axis(cfg.axis_map[XBOX_AXIS_Y2], -msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      send_axis(cfg.axis_map[XBOX_AXIS_TRIGGER], (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      send_button(cfg.btn_map[XBOX_BTN_LT], msg.lt);
      send_button(cfg.btn_map[XBOX_BTN_RT], msg.rt);
    }
  else
    {
      send_axis(cfg.axis_map[XBOX_AXIS_LT], msg.lt);
      send_axis(cfg.axis_map[XBOX_AXIS_RT], msg.rt);
    }
  
  if (cfg.dpad_as_button && !cfg.dpad_only)
    {
      send_button(BTN_BASE,  msg.dpad_up);
      send_button(BTN_BASE2, msg.dpad_down);
      send_button(BTN_BASE3, msg.dpad_left);
      send_button(BTN_BASE4, msg.dpad_right);
    }
  else
    {
      uint16_t dpad_x = cfg.axis_map[XBOX_AXIS_DPAD_X];
      uint16_t dpad_y = cfg.axis_map[XBOX_AXIS_DPAD_Y];
      
      if (cfg.dpad_only) // FIXME: Implement via ui-buttonmap
        {
          dpad_x = cfg.axis_map[XBOX_AXIS_X1];
          dpad_y = cfg.axis_map[XBOX_AXIS_Y1];
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

  send_button(cfg.btn_map[XBOX_BTN_A], msg.a);
  send_button(cfg.btn_map[XBOX_BTN_B], msg.b);
  send_button(cfg.btn_map[XBOX_BTN_X], msg.x);
  send_button(cfg.btn_map[XBOX_BTN_Y], msg.y);

  send_axis(cfg.axis_map[XBOX_AXIS_X1], msg.x1);
  send_axis(cfg.axis_map[XBOX_AXIS_Y1], msg.y1);

  send_axis(cfg.axis_map[XBOX_AXIS_X2], msg.x2);
  send_axis(cfg.axis_map[XBOX_AXIS_Y2], msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      send_axis(cfg.axis_map[XBOX_AXIS_TRIGGER], (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      send_button(cfg.btn_map[XBOX_BTN_LT], msg.lt);
      send_button(cfg.btn_map[XBOX_BTN_RT], msg.rt);
    }
  else
    {
      send_axis(cfg.axis_map[XBOX_AXIS_LT], msg.lt);
      send_axis(cfg.axis_map[XBOX_AXIS_RT],   msg.rt);
    }

  if (cfg.dpad_as_button)
    {
      send_button(cfg.btn_map[XBOX_DPAD_UP],    msg.dpad_up);
      send_button(cfg.btn_map[XBOX_DPAD_DOWN],  msg.dpad_down);
      send_button(cfg.btn_map[XBOX_DPAD_LEFT],  msg.dpad_left);
      send_button(cfg.btn_map[XBOX_DPAD_RIGHT], msg.dpad_right);
    }
  else
    {
      if (msg.dpad_up)
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], -1);
        }
      else if (msg.dpad_down)
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], 1);
        }
      else
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], 0);
        }

      if (msg.dpad_left)
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], -1);
        }
      else if (msg.dpad_right)
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], 1);
        }
      else
        {
          send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], 0);
        }
    }
}

void
uInput::send(Xbox360GuitarMsg& msg)
{
  send_button(cfg.btn_map[XBOX_DPAD_UP],    msg.dpad_up);
  send_button(cfg.btn_map[XBOX_DPAD_DOWN],  msg.dpad_down);
  send_button(cfg.btn_map[XBOX_DPAD_LEFT],  msg.dpad_left);
  send_button(cfg.btn_map[XBOX_DPAD_RIGHT], msg.dpad_right);

  send_button(cfg.btn_map[XBOX_BTN_START], msg.start);
  send_button(cfg.btn_map[XBOX_BTN_GUIDE], msg.guide);
  send_button(cfg.btn_map[XBOX_BTN_BACK],  msg.back);

  send_button(BTN_1, msg.green);
  send_button(BTN_2, msg.red);
  send_button(BTN_3, msg.yellow);
  send_button(BTN_4, msg.blue);
  send_button(BTN_5, msg.orange);

  send_axis(cfg.axis_map[XBOX_AXIS_X1], msg.whammy);
  send_axis(cfg.axis_map[XBOX_AXIS_Y1], msg.tilt);
}

/* EOF */
