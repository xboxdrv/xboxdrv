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
  force_feedback    = false;

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
      if ((fd = open(uinput_filename[i], O_RDWR | O_NDELAY)) >= 0)
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
  // LED
  ioctl(fd, UI_SET_EVBIT, EV_LED);
  ioctl(fd, UI_SET_LEDBIT, LED_MISC);

  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  ioctl(fd, UI_SET_EVBIT, EV_KEY);

  if (cfg.force_feedback)
    {
      ioctl(fd, UI_SET_EVBIT, EV_FF);
      ioctl(fd, UI_SET_FFBIT, FF_RUMBLE);
      ioctl(fd, UI_SET_FFBIT, FF_PERIODIC);

      // More stuff, only for testing
      ioctl(fd, UI_SET_FFBIT, FF_CONSTANT);
      ioctl(fd, UI_SET_FFBIT, FF_SPRING);
      ioctl(fd, UI_SET_FFBIT, FF_FRICTION);
      ioctl(fd, UI_SET_FFBIT, FF_DAMPER);
      ioctl(fd, UI_SET_FFBIT, FF_INERTIA);
      ioctl(fd, UI_SET_FFBIT, FF_RAMP);
    }

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

  if (cfg.force_feedback)
    uinp.ff_effects_max = 64; 

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

std::ostream& operator<<(std::ostream& out, const struct ff_envelope& envelope)
{
  out << "attack_length: " << envelope.attack_length
      << " attack_level: " << envelope.attack_level
      << " fade_length: " << envelope.fade_length
      << " fade_level: " << envelope.fade_level;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_replay& replay)
{
  out << "length: " << replay.length << " delay: " << replay.delay;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_trigger& trigger)
{
  out << "button: " << trigger.button << " interval: " << trigger.interval;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_effect& effect)
{
  switch (effect.type)
    {
      case FF_CONSTANT:
        out << "FF_CONSTANT "
            << "level: " << effect.u.constant.level
            << " envelope: { " << effect.u.constant.envelope << " }";
        break;

      case FF_PERIODIC:
        out << "FF_PERIODIC"
            << " waveform: " << effect.u.periodic.waveform
            << " period: " << effect.u.periodic.period
            << " magnitude: " << effect.u.periodic.magnitude
            << " offset: " << effect.u.periodic.offset
            << " phase: " << effect.u.periodic.phase
            << " envelope: { " << effect.u.periodic.envelope << " }";
        break;

      case FF_RAMP:
        out << "FF_RAMP " 
            << "start_level: " << effect.u.ramp.start_level
            << "end_level: " << effect.u.ramp.end_level
            << "envelope: { " <<  effect.u.ramp.envelope << " }";
        break;

      case FF_SPRING:
        out << "FF_SPRING";
        break;

      case FF_FRICTION:
        out << "FF_FRICTION";
        break;

      case FF_DAMPER:
        out << "FF_DAMPER";
        break;

      case FF_RUMBLE:
        out << "FF_RUMBLE: "
            << "strong_magnitude: " << effect.u.rumble.strong_magnitude
            << " weak_magnitude: " << effect.u.rumble.weak_magnitude;
        break;

      case FF_INERTIA:
        out << "FF_INERTIA";
        break;

      case FF_CUSTOM:
        out << "FF_CUSTOM";
        break;

      default:
        out << "FF_<unknown>";
        break;
    }

  out << "\n";
  out << "direction: " << effect.direction << "\n";
  out << "replay: " << effect.replay << "\n";
  out << "trigger: " << effect.trigger << "\n";

  return out;
}

void
uInput::update()
{
  if (cfg.force_feedback)
    {
      struct input_event ev;

      int ret = read(fd, &ev, sizeof(ev));
      if (ret < 0)
        {
          if (errno != EAGAIN)
            std::cout << "Error: " << strerror(errno) << " " << ret << std::endl;
        }
      else if (ret == sizeof(ev))
        { // successful read
          std::cout << "type: " << ev.type << " code: " << ev.code << " value: " << ev.value << std::endl;

          switch(ev.type)
            {
              case EV_LED:
                if (ev.code == LED_MISC)
                  {
                    std::cout << "Set LED status: " << ev.value << std::endl;
                  }
                break;

              case EV_FF:
                std::cout << "EV_FF: playing effect: effect_id = " << ev.code << " value: " << ev.value << std::endl;
                break;

              case EV_UINPUT:
                switch (ev.code)
                  {
                    case UI_FF_UPLOAD:
                      {
                        struct uinput_ff_upload upload;
                        memset(&upload, 0, sizeof(upload));

                        // *VERY* important, without this you
                        // break the kernel and have to reboot due
                        // to dead hanging process
                        upload.request_id = ev.value;

                        ioctl(fd, UI_BEGIN_FF_UPLOAD, &upload);

                        std::cout << "XXX FF_UPLOAD: rumble upload:"
                                  << " effect_id: " << upload.effect.id
                                  << " effect_type: " << upload.effect.type
                                  << std::endl;
                        std::cout << "EFFECT: " << upload.effect << std::endl;

                        upload.retval = 0;
                            
                        ioctl(fd, UI_END_FF_UPLOAD, &upload);
                      }
                      break;

                    case UI_FF_ERASE:
                      {
                        struct uinput_ff_erase erase;
                        memset(&erase, 0, sizeof(erase));

                        // *VERY* important, without this you
                        // break the kernel and have to reboot due
                        // to dead hanging process
                        erase.request_id = ev.value;

                        ioctl(fd, UI_BEGIN_FF_ERASE, &erase);

                        std::cout << "FF_ERASE: rumble erase: effect_id = " << erase.effect_id << std::endl;
                        erase.retval = 0; // FIXME: is this used?
                            
                        ioctl(fd, UI_END_FF_ERASE, &erase);
                      }
                      break;

                    default: 
                      std::cout << "Unhandled event code read" << std::endl;
                      break;
                  }
                break;

              default:
                std::cout << "Unhandled event type read: " << ev.type << std::endl;
                break;
            }
          std::cout << "--------------------------------" << std::endl;
        }
      else
        {
          std::cout << "uInput::update: short read: " << ret << std::endl;
        }
    }
}

/* EOF */
