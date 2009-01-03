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
  btn_map[XBOX_BTN_START] = Event::create(EV_KEY, BTN_START);
  btn_map[XBOX_BTN_GUIDE] = Event::create(EV_KEY, BTN_MODE);
  btn_map[XBOX_BTN_BACK]  = Event::create(EV_KEY, BTN_SELECT);

  btn_map[XBOX_BTN_A] = Event::create(EV_KEY, BTN_A);
  btn_map[XBOX_BTN_B] = Event::create(EV_KEY, BTN_B);
  btn_map[XBOX_BTN_X] = Event::create(EV_KEY, BTN_X);
  btn_map[XBOX_BTN_Y] = Event::create(EV_KEY, BTN_Y);

  btn_map[XBOX_BTN_GREEN]  = Event::create(EV_KEY, BTN_0);
  btn_map[XBOX_BTN_RED]    = Event::create(EV_KEY, BTN_1);
  btn_map[XBOX_BTN_YELLOW] = Event::create(EV_KEY, BTN_2);
  btn_map[XBOX_BTN_BLUE]   = Event::create(EV_KEY, BTN_3);
  btn_map[XBOX_BTN_ORANGE] = Event::create(EV_KEY, BTN_4);

  btn_map[XBOX_BTN_WHITE] = Event::create(EV_KEY, BTN_TL);
  btn_map[XBOX_BTN_BLACK] = Event::create(EV_KEY, BTN_TR);

  btn_map[XBOX_BTN_LB] = Event::create(EV_KEY, BTN_TL);
  btn_map[XBOX_BTN_RB] = Event::create(EV_KEY, BTN_TR);

  btn_map[XBOX_BTN_LT] = Event::create(EV_KEY, BTN_TL2);
  btn_map[XBOX_BTN_RT] = Event::create(EV_KEY, BTN_TR2);

  btn_map[XBOX_BTN_THUMB_L] = Event::create(EV_KEY, BTN_THUMBL);
  btn_map[XBOX_BTN_THUMB_R] = Event::create(EV_KEY, BTN_THUMBR);
  
  btn_map[XBOX_DPAD_UP]    = Event::create(EV_KEY, BTN_BASE);
  btn_map[XBOX_DPAD_DOWN]  = Event::create(EV_KEY, BTN_BASE2);
  btn_map[XBOX_DPAD_LEFT]  = Event::create(EV_KEY, BTN_BASE3);
  btn_map[XBOX_DPAD_RIGHT] = Event::create(EV_KEY, BTN_BASE4);

  // Axis Mapping
  axis_map[XBOX_AXIS_X1]      = Event::create(EV_ABS, ABS_X); 
  axis_map[XBOX_AXIS_Y1]      = Event::create(EV_ABS, ABS_Y); 
  axis_map[XBOX_AXIS_X2]      = Event::create(EV_ABS, ABS_RX);
  axis_map[XBOX_AXIS_Y2]      = Event::create(EV_ABS, ABS_RY);
  axis_map[XBOX_AXIS_LT]      = Event::create(EV_ABS, ABS_GAS);
  axis_map[XBOX_AXIS_RT]      = Event::create(EV_ABS, ABS_BRAKE); 
  axis_map[XBOX_AXIS_TRIGGER] = Event::create(EV_ABS, ABS_Z); 
  axis_map[XBOX_AXIS_DPAD_X]  = Event::create(EV_ABS, ABS_HAT0X);
  axis_map[XBOX_AXIS_DPAD_Y]  = Event::create(EV_ABS, ABS_HAT0Y);
}

uInput::uInput(GamepadType type, uInputCfg config_)
  :cfg(config_)
{
  joy_uinput = std::auto_ptr<LinuxUinput>(new LinuxUinput());
  key_uinput = std::auto_ptr<LinuxUinput>(new LinuxUinput());

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

  joy_uinput->finish("Xbox Gamepad (userspace driver)");
  key_uinput->finish("Xbox Gamepad - Keyboard Emulation (userspace driver)");
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

  add_abs(cfg.axis_map[XBOX_AXIS_X1], -32768, 32767);
  add_abs(cfg.axis_map[XBOX_AXIS_Y1], -32768, 32767);

  if (!cfg.dpad_only)
    {  
      add_abs(cfg.axis_map[XBOX_AXIS_X2], -32768, 32767);
      add_abs(cfg.axis_map[XBOX_AXIS_Y2], -32768, 32767);
    }

  if (cfg.trigger_as_button)
    {
      add_key(cfg.btn_map[XBOX_BTN_LT]);
      add_key(cfg.btn_map[XBOX_BTN_RT]);
    }
  else if (cfg.trigger_as_zaxis)
    {
      add_abs(cfg.axis_map[XBOX_AXIS_TRIGGER], -255, 255);
    }
  else
    {
      add_abs(cfg.axis_map[XBOX_AXIS_LT], 0, 255);
      add_abs(cfg.axis_map[XBOX_AXIS_RT], 0, 255);
    }

  if (!cfg.dpad_only)
    {
      if (!cfg.dpad_as_button)
        {
          add_abs(cfg.axis_map[XBOX_AXIS_DPAD_X], -1, 1);
          add_abs(cfg.axis_map[XBOX_AXIS_DPAD_Y], -1, 1);
        }
      else
        {
          add_key(cfg.btn_map[XBOX_DPAD_UP]);
          add_key(cfg.btn_map[XBOX_DPAD_DOWN]);
          add_key(cfg.btn_map[XBOX_DPAD_LEFT]);
          add_key(cfg.btn_map[XBOX_DPAD_RIGHT]);
        }
    }

  add_key(cfg.btn_map[XBOX_BTN_START]);
  add_key(cfg.btn_map[XBOX_BTN_BACK]);
        
  if (type == GAMEPAD_XBOX360 || type == GAMEPAD_XBOX360_WIRELESS)
    add_key(cfg.btn_map[XBOX_BTN_GUIDE]);

  add_key(cfg.btn_map[XBOX_BTN_A]);
  add_key(cfg.btn_map[XBOX_BTN_B]);
  add_key(cfg.btn_map[XBOX_BTN_X]);
  add_key(cfg.btn_map[XBOX_BTN_Y]);

  add_key(cfg.btn_map[XBOX_BTN_LB]);
  add_key(cfg.btn_map[XBOX_BTN_RB]);

  add_key(cfg.btn_map[XBOX_BTN_THUMB_L]);
  add_key(cfg.btn_map[XBOX_BTN_THUMB_R]);

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
  add_abs(cfg.axis_map[XBOX_AXIS_X1], -32768, 32767);
  add_abs(cfg.axis_map[XBOX_AXIS_Y1], -32768, 32767);

  // Dpad
  add_key(cfg.btn_map[XBOX_DPAD_UP]);
  add_key(cfg.btn_map[XBOX_DPAD_DOWN]);
  add_key(cfg.btn_map[XBOX_DPAD_LEFT]);
  add_key(cfg.btn_map[XBOX_DPAD_RIGHT]);

  // Base
  add_key(cfg.btn_map[XBOX_BTN_START]);
  add_key(cfg.btn_map[XBOX_BTN_BACK]);
  add_key(cfg.btn_map[XBOX_BTN_GUIDE]);

  // Fret button
  add_key(cfg.btn_map[XBOX_BTN_GREEN]);
  add_key(cfg.btn_map[XBOX_BTN_RED]);
  add_key(cfg.btn_map[XBOX_BTN_BLUE]);
  add_key(cfg.btn_map[XBOX_BTN_YELLOW]);
  add_key(cfg.btn_map[XBOX_BTN_ORANGE]);
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
  send_button(XBOX_BTN_THUMB_L, msg.thumb_l);
  send_button(XBOX_BTN_THUMB_R, msg.thumb_r);

  send_button(XBOX_BTN_LB, msg.lb);
  send_button(XBOX_BTN_RB, msg.rb);

  send_button(XBOX_BTN_START, msg.start);
  send_button(XBOX_BTN_GUIDE, msg.guide);
  send_button(XBOX_BTN_BACK, msg.back);

  send_button(XBOX_BTN_A, msg.a);
  send_button(XBOX_BTN_B, msg.b);
  send_button(XBOX_BTN_X, msg.x);
  send_button(XBOX_BTN_Y, msg.y);

  send_axis(XBOX_AXIS_X1, msg.x1);
  send_axis(XBOX_AXIS_Y1, -msg.y1);

  send_axis(XBOX_AXIS_X2, msg.x2);
  send_axis(XBOX_AXIS_Y2, -msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      send_axis(XBOX_AXIS_TRIGGER, (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      send_button(XBOX_BTN_LT, msg.lt);
      send_button(XBOX_BTN_RT, msg.rt);
    }
  else
    {
      send_axis(XBOX_AXIS_LT, msg.lt);
      send_axis(XBOX_AXIS_RT, msg.rt);
    }
  
  if (cfg.dpad_as_button && !cfg.dpad_only)
    {
      send_button(XBOX_DPAD_UP,   msg.dpad_up);
      send_button(XBOX_DPAD_DOWN, msg.dpad_down);
      send_button(XBOX_DPAD_LEFT, msg.dpad_left);
      send_button(XBOX_DPAD_RIGHT, msg.dpad_right);
    }
  else
    {
      int dpad_x = XBOX_AXIS_DPAD_X;
      int dpad_y = XBOX_AXIS_DPAD_Y;
      
      if (cfg.dpad_only) // FIXME: Implement via ui-buttonmap
        {
          dpad_x = XBOX_AXIS_X1;
          dpad_y = XBOX_AXIS_Y1;
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
  send_button(XBOX_BTN_THUMB_L, msg.thumb_l);
  send_button(XBOX_BTN_THUMB_R, msg.thumb_r);

  send_button(XBOX_BTN_WHITE, msg.white);
  send_button(XBOX_BTN_BLACK, msg.black);

  send_button(XBOX_BTN_START, msg.start);
  send_button(XBOX_BTN_BACK,  msg.back);

  send_button(XBOX_BTN_A, msg.a);
  send_button(XBOX_BTN_B, msg.b);
  send_button(XBOX_BTN_X, msg.x);
  send_button(XBOX_BTN_Y, msg.y);

  send_axis(XBOX_AXIS_X1, msg.x1);
  send_axis(XBOX_AXIS_Y1, msg.y1);

  send_axis(XBOX_AXIS_X2, msg.x2);
  send_axis(XBOX_AXIS_Y2, msg.y2);

  if (cfg.trigger_as_zaxis)
    {
      send_axis(XBOX_AXIS_TRIGGER, (int(msg.rt) - int(msg.lt)));
    }
  else if (cfg.trigger_as_button)
    {
      send_button(XBOX_BTN_LT, msg.lt);
      send_button(XBOX_BTN_RT, msg.rt);
    }
  else
    {
      send_axis(XBOX_AXIS_LT, msg.lt);
      send_axis(XBOX_AXIS_RT,   msg.rt);
    }

  if (cfg.dpad_as_button)
    {
      send_button(XBOX_DPAD_UP,   msg.dpad_up);
      send_button(XBOX_DPAD_DOWN, msg.dpad_down);
      send_button(XBOX_DPAD_LEFT, msg.dpad_left);
      send_button(XBOX_DPAD_RIGHT, msg.dpad_right);
    }
  else
    {
      if (msg.dpad_up)
        {
          send_axis(XBOX_AXIS_DPAD_Y, -1);
        }
      else if (msg.dpad_down)
        {
          send_axis(XBOX_AXIS_DPAD_Y, 1);
        }
      else
        {
          send_axis(XBOX_AXIS_DPAD_Y, 0);
        }

      if (msg.dpad_left)
        {
          send_axis(XBOX_AXIS_DPAD_X, -1);
        }
      else if (msg.dpad_right)
        {
          send_axis(XBOX_AXIS_DPAD_X, 1);
        }
      else
        {
          send_axis(XBOX_AXIS_DPAD_X, 0);
        }
    }
}

void
uInput::send(Xbox360GuitarMsg& msg)
{
  send_button(XBOX_DPAD_UP,   msg.dpad_up);
  send_button(XBOX_DPAD_DOWN, msg.dpad_down);
  send_button(XBOX_DPAD_LEFT, msg.dpad_left);
  send_button(XBOX_DPAD_RIGHT, msg.dpad_right);

  send_button(XBOX_BTN_START, msg.start);
  send_button(XBOX_BTN_GUIDE, msg.guide);
  send_button(XBOX_BTN_BACK,  msg.back);

  send_button(XBOX_BTN_GREEN,  msg.green);
  send_button(XBOX_BTN_RED,    msg.red);
  send_button(XBOX_BTN_YELLOW, msg.yellow);
  send_button(XBOX_BTN_BLUE,   msg.blue);
  send_button(XBOX_BTN_ORANGE, msg.orange);

  send_axis(XBOX_AXIS_X1, msg.whammy);
  send_axis(XBOX_AXIS_Y1, msg.tilt);
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

void
uInput::send_button(int code, int32_t value)
{
  const Event& event = cfg.btn_map[code];
  
  if (event.code < 256)
    key_uinput->send_button(event.code, value);
  else
    joy_uinput->send_button(event.code, value);
}

void
uInput::send_axis(int code, int32_t value)
{
  const Event& event = cfg.axis_map[code];

  joy_uinput->send_axis(event.code, value);
}

void
uInput::add_abs(const Event& event, int min, int max)
{
  joy_uinput->add_abs(event.code, min, max);
}

void
uInput::add_key(const Event& event)
{
  if (event.code < 256)
    key_uinput->add_key(event.code);
  else
    joy_uinput->add_key(event.code);
}

/* EOF */
