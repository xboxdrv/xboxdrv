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
  :cfg(config_)
{
  uinput = std::auto_ptr<LinuxUinput>(new LinuxUinput());

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

  uinput->finish();
}

void
uInput::setup_xbox360_gamepad(GamepadType type)
{
  // LED
  //ioctl(fd, UI_SET_EVBIT, EV_LED);
  //ioctl(fd, UI_SET_LEDBIT, LED_MISC);

  if (cfg.force_feedback)
    {
//       ioctl(fd, UI_SET_EVBIT, EV_FF);
//       ioctl(fd, UI_SET_FFBIT, FF_RUMBLE);
//       ioctl(fd, UI_SET_FFBIT, FF_PERIODIC);

//       // More stuff, only for testing
//       ioctl(fd, UI_SET_FFBIT, FF_CONSTANT);
//       ioctl(fd, UI_SET_FFBIT, FF_SPRING);
//       ioctl(fd, UI_SET_FFBIT, FF_FRICTION);
//       ioctl(fd, UI_SET_FFBIT, FF_DAMPER);
//       ioctl(fd, UI_SET_FFBIT, FF_INERTIA);
//       ioctl(fd, UI_SET_FFBIT, FF_RAMP);
    }

  uinput->add_abs(cfg.axis_map[XBOX_AXIS_X1], -32768, 32767);
  uinput->add_abs(cfg.axis_map[XBOX_AXIS_Y1], -32768, 32767);

  if (!cfg.dpad_only)
    {  
      uinput->add_abs(cfg.axis_map[XBOX_AXIS_X2], -32768, 32767);
      uinput->add_abs(cfg.axis_map[XBOX_AXIS_Y2], -32768, 32767);
    }

  if (cfg.trigger_as_button)
    {
      uinput->add_key(cfg.btn_map[XBOX_BTN_LT]);
      uinput->add_key(cfg.btn_map[XBOX_BTN_RT]);
    }
  else if (cfg.trigger_as_zaxis)
    {
      uinput->add_abs(cfg.axis_map[XBOX_AXIS_TRIGGER], -255, 255);
    }
  else
    {
      uinput->add_abs(cfg.axis_map[XBOX_AXIS_LT], 0, 255);
      uinput->add_abs(cfg.axis_map[XBOX_AXIS_RT], 0, 255);
    }

  if (!cfg.dpad_only)
    {
      if (!cfg.dpad_as_button)
        {
          uinput->add_abs(cfg.axis_map[XBOX_AXIS_DPAD_X], -1, 1);
          uinput->add_abs(cfg.axis_map[XBOX_AXIS_DPAD_Y], -1, 1);
        }
      else
        {
          uinput->add_key(cfg.btn_map[XBOX_DPAD_UP]);
          uinput->add_key(cfg.btn_map[XBOX_DPAD_DOWN]);
          uinput->add_key(cfg.btn_map[XBOX_DPAD_LEFT]);
          uinput->add_key(cfg.btn_map[XBOX_DPAD_RIGHT]);
        }
    }

  uinput->add_key(cfg.btn_map[XBOX_BTN_START]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_BACK]);
        
  if (type == GAMEPAD_XBOX360 || type == GAMEPAD_XBOX360_WIRELESS)
    uinput->add_key(cfg.btn_map[XBOX_BTN_GUIDE]);

  uinput->add_key(cfg.btn_map[XBOX_BTN_A]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_B]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_X]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_Y]);

  uinput->add_key(cfg.btn_map[XBOX_BTN_LB]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_RB]);

  uinput->add_key(cfg.btn_map[XBOX_BTN_THUMB_L]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_THUMB_R]);

  struct uinput_user_dev uinp;
  memset(&uinp,0,sizeof(uinp));
  
  strncpy(uinp.name, "Xbox Gamepad (userspace driver)", UINPUT_MAX_NAME_SIZE);

  // if (cfg.force_feedback)
  // uinp.ff_effects_max = 64; 
}

void
uInput::setup_xbox360_guitar()
{
  // Whammy and Tilt
  uinput->add_abs(cfg.axis_map[XBOX_AXIS_X1], -32768, 32767);
  uinput->add_abs(cfg.axis_map[XBOX_AXIS_Y1], -32768, 32767);

  // Dpad
  uinput->add_key(cfg.btn_map[XBOX_DPAD_UP]);
  uinput->add_key(cfg.btn_map[XBOX_DPAD_DOWN]);
  uinput->add_key(cfg.btn_map[XBOX_DPAD_LEFT]);
  uinput->add_key(cfg.btn_map[XBOX_DPAD_RIGHT]);

  // Base
  uinput->add_key(cfg.btn_map[XBOX_BTN_START]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_BACK]);
  uinput->add_key(cfg.btn_map[XBOX_BTN_GUIDE]);

  // Fret button
  uinput->add_key(BTN_1);
  uinput->add_key(BTN_2);
  uinput->add_key(BTN_3);
  uinput->add_key(BTN_4);
  uinput->add_key(BTN_5);
}

uInput::~uInput()
{
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
  uinput->send_button(BTN_THUMBL,  msg.thumb_l);
  uinput->send_button(BTN_THUMBR,  msg.thumb_r);

  uinput->send_button(BTN_TL,  msg.lb);
  uinput->send_button(BTN_TR,  msg.rb);

  uinput->send_button(cfg.btn_map[XBOX_BTN_START], msg.start);
  uinput->send_button(cfg.btn_map[XBOX_BTN_GUIDE], msg.guide);
  uinput->send_button(cfg.btn_map[XBOX_BTN_BACK],  msg.back);

  uinput->send_button(cfg.btn_map[XBOX_BTN_A], msg.a);
  uinput->send_button(cfg.btn_map[XBOX_BTN_B], msg.b);
  uinput->send_button(cfg.btn_map[XBOX_BTN_X], msg.x);
  uinput->send_button(cfg.btn_map[XBOX_BTN_Y], msg.y);

  uinput->send_axis(cfg.axis_map[XBOX_AXIS_X1],  msg.x1);
  uinput->send_axis(cfg.axis_map[XBOX_AXIS_Y1], -msg.y1);

  uinput->send_axis(cfg.axis_map[XBOX_AXIS_X2],  msg.x2);
  uinput->send_axis(cfg.axis_map[XBOX_AXIS_Y2], -msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_TRIGGER], (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      uinput->send_button(cfg.btn_map[XBOX_BTN_LT], msg.lt);
      uinput->send_button(cfg.btn_map[XBOX_BTN_RT], msg.rt);
    }
  else
    {
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_LT], msg.lt);
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_RT], msg.rt);
    }
  
  if (cfg.dpad_as_button && !cfg.dpad_only)
    {
      uinput->send_button(BTN_BASE,  msg.dpad_up);
      uinput->send_button(BTN_BASE2, msg.dpad_down);
      uinput->send_button(BTN_BASE3, msg.dpad_left);
      uinput->send_button(BTN_BASE4, msg.dpad_right);
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
          uinput->send_axis(dpad_y, -1);
        }
      else if (msg.dpad_down)
        {
          uinput->send_axis(dpad_y, 1);
        }
      else
        {
          uinput->send_axis(dpad_y, 0);
        }

      if (msg.dpad_left)
        {
          uinput->send_axis(dpad_x, -1);
        }
      else if (msg.dpad_right)
        {
          uinput->send_axis(dpad_x, 1);
        }
      else
        {
          uinput->send_axis(dpad_x, 0);
        }
    }
}

void
uInput::send(XboxMsg& msg)
{
  uinput->send_button(BTN_THUMBL,  msg.thumb_l);
  uinput->send_button(BTN_THUMBR,  msg.thumb_r);

  uinput->send_button(BTN_TL,  msg.white);
  uinput->send_button(BTN_TR,  msg.black);

  uinput->send_button(BTN_START,  msg.start);
  uinput->send_button(BTN_SELECT, msg.back);

  uinput->send_button(cfg.btn_map[XBOX_BTN_A], msg.a);
  uinput->send_button(cfg.btn_map[XBOX_BTN_B], msg.b);
  uinput->send_button(cfg.btn_map[XBOX_BTN_X], msg.x);
  uinput->send_button(cfg.btn_map[XBOX_BTN_Y], msg.y);

  uinput->send_axis(cfg.axis_map[XBOX_AXIS_X1], msg.x1);
  uinput->send_axis(cfg.axis_map[XBOX_AXIS_Y1], msg.y1);

  uinput->send_axis(cfg.axis_map[XBOX_AXIS_X2], msg.x2);
  uinput->send_axis(cfg.axis_map[XBOX_AXIS_Y2], msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_TRIGGER], (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      uinput->send_button(cfg.btn_map[XBOX_BTN_LT], msg.lt);
      uinput->send_button(cfg.btn_map[XBOX_BTN_RT], msg.rt);
    }
  else
    {
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_LT], msg.lt);
      uinput->send_axis(cfg.axis_map[XBOX_AXIS_RT],   msg.rt);
    }

  if (cfg.dpad_as_button)
    {
      uinput->send_button(cfg.btn_map[XBOX_DPAD_UP],    msg.dpad_up);
      uinput->send_button(cfg.btn_map[XBOX_DPAD_DOWN],  msg.dpad_down);
      uinput->send_button(cfg.btn_map[XBOX_DPAD_LEFT],  msg.dpad_left);
      uinput->send_button(cfg.btn_map[XBOX_DPAD_RIGHT], msg.dpad_right);
    }
  else
    {
      if (msg.dpad_up)
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], -1);
        }
      else if (msg.dpad_down)
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], 1);
        }
      else
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_Y], 0);
        }

      if (msg.dpad_left)
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], -1);
        }
      else if (msg.dpad_right)
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], 1);
        }
      else
        {
          uinput->send_axis(cfg.axis_map[XBOX_AXIS_DPAD_X], 0);
        }
    }
}

void
uInput::send(Xbox360GuitarMsg& msg)
{
  uinput->send_button(cfg.btn_map[XBOX_DPAD_UP],    msg.dpad_up);
  uinput->send_button(cfg.btn_map[XBOX_DPAD_DOWN],  msg.dpad_down);
  uinput->send_button(cfg.btn_map[XBOX_DPAD_LEFT],  msg.dpad_left);
  uinput->send_button(cfg.btn_map[XBOX_DPAD_RIGHT], msg.dpad_right);

  uinput->send_button(cfg.btn_map[XBOX_BTN_START], msg.start);
  uinput->send_button(cfg.btn_map[XBOX_BTN_GUIDE], msg.guide);
  uinput->send_button(cfg.btn_map[XBOX_BTN_BACK],  msg.back);

  uinput->send_button(BTN_1, msg.green);
  uinput->send_button(BTN_2, msg.red);
  uinput->send_button(BTN_3, msg.yellow);
  uinput->send_button(BTN_4, msg.blue);
  uinput->send_button(BTN_5, msg.orange);

  uinput->send_axis(cfg.axis_map[XBOX_AXIS_X1], msg.whammy);
  uinput->send_axis(cfg.axis_map[XBOX_AXIS_Y1], msg.tilt);
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
#if 0
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
                    // FIXME: implement this
                    std::cout << "unimplemented: Set LED status: " << ev.value << std::endl;
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
#endif
}

/* EOF */
