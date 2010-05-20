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

#include <algorithm>
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
#include "uinput_deviceid.hpp"

bool is_keyboard_event(const ButtonEvent& event)
{
  if (event.type == EV_KEY && uInput::is_keyboard_button(event.code))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool is_mouse_event(const ButtonEvent& event)
{
  if (event.type == EV_KEY && uInput::is_mouse_button(event.code))
  {
    return true;
  }
  else if (event.type == EV_REL)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
uInput::need_keyboard_device()
{
  if (cfg.btn_map.contains(is_keyboard_event))
  {
    return true;
  }

  for(int i = 0; i < XBOX_AXIS_MAX; ++i)
  {
    if (cfg.axis_map[i].type == EV_KEY &&
        (is_keyboard_button(cfg.axis_map[i].code) ||
         is_keyboard_button(cfg.axis_map[i].key.secondary_code)))
    {
      return true;
    }
  }

  return false;
}

bool
uInput::need_mouse_device()
{
  if (cfg.btn_map.contains(is_mouse_event))
  {
    return true;
  }

  for(int i = 0; i < XBOX_AXIS_MAX; ++i)
  {
    if (cfg.axis_map[i].type == EV_KEY &&
        (is_mouse_button(cfg.axis_map[i].code) ||
         is_mouse_button(cfg.axis_map[i].key.secondary_code)))
    {
      return true;
    }
    else if (cfg.axis_map[i].type == EV_REL)
    {
      return true;
    }
  }

  return false;
}

bool
uInput::need_joystick_device()
{
  // FIXME: Implement me
  return true;
}

bool
uInput::is_mouse_button(int ev_code)
{
  return  (ev_code >= BTN_MOUSE && ev_code <= BTN_TASK);
}

bool
uInput::is_keyboard_button(int ev_code)
{
  return (ev_code < 256);
}

uInput::uInput(const XPadDevice& dev, uInputCfg config_) :
  m_dev(dev),
  uinput_devs(),
  cfg(config_),
  rel_axis(),
  rel_button()
{
  std::fill_n(axis_state,   static_cast<int>(XBOX_AXIS_MAX), 0);
  std::fill_n(button_state, static_cast<int>(XBOX_BTN_MAX),  false);

  if (cfg.force_feedback)
  {
    create_uinput_device(0);
  }

  for(ButtonMap::iterator i = cfg.btn_map.begin(); i != cfg.btn_map.end(); ++i)
  {
    (*i).device_id = create_uinput_device(*i);
  }

  for(int i = 0; i < XBOX_AXIS_MAX; ++i)
  {
    cfg.axis_map[i].device_id = create_uinput_device(cfg.axis_map[i]);
  }

  switch(dev.type)
  {
    case GAMEPAD_XBOX360:
    case GAMEPAD_XBOX:
    case GAMEPAD_XBOX360_WIRELESS:
    case GAMEPAD_FIRESTORM:
    case GAMEPAD_FIRESTORM_VSB:
      setup_xbox360_gamepad(dev.type);
      break;

    case GAMEPAD_XBOX360_GUITAR:
      setup_xbox360_guitar();
      break;

    default:
      std::cout << "Unhandled type: " << dev.type << std::endl;
      exit(EXIT_FAILURE);
      break;
  }

  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->finish();
  }
}

int
uInput::create_uinput_device(int device_id)
{
  uInputDevs::iterator it = uinput_devs.find(device_id);
  if (it != uinput_devs.end())
  {
    // device already exist, which is fine    
    return device_id;
  }
  else
  {
    std::ostringstream dev_name;
    dev_name << cfg.device_name;

    if (device_id == DEVICEID_MOUSE)
    {
      dev_name << " - Mouse Emulation";
    }
    else if (device_id == DEVICEID_KEYBOARD)
    {
      dev_name << " - Keyboard Emulation";
    }
    else if (device_id > 0)
    {
      dev_name << " - 2" << device_id+1;
    }

    boost::shared_ptr<LinuxUinput> dev(new LinuxUinput(dev_name.str(), m_dev.idVendor, m_dev.idProduct));
    uinput_devs.insert(std::pair<int, boost::shared_ptr<LinuxUinput> >(device_id, dev));

    std::cout << "Creating uinput device: device_id: " << device_id << ", dev_name: " << dev_name.str() << std::endl;

    return device_id;
  }
}

int
uInput::create_uinput_device(const AxisEvent& event)
{
  if (event.is_valid())
  {
    if (event.device_id == DEVICEID_AUTO)
    {
      if (cfg.extra_devices)
      {
        if (event.type == EV_KEY)
        {
          if (is_mouse_button(event.code) || is_mouse_button(event.key.secondary_code))
          {
            return create_uinput_device(DEVICEID_MOUSE);
          }
          else if (is_keyboard_button(event.code) || is_keyboard_button(event.key.secondary_code))
          {
            return create_uinput_device(DEVICEID_KEYBOARD);
          }
          else
          {
            return create_uinput_device(DEVICEID_JOYSTICK);
          }
        }
        else if (event.type == EV_REL)
        {
          return create_uinput_device(DEVICEID_MOUSE);
        }
        else
        {
          return create_uinput_device(DEVICEID_JOYSTICK);
        }
      }
      else
      {
        return create_uinput_device(DEVICEID_JOYSTICK);
      }
    }
    else
    {
      return create_uinput_device(event.device_id);
    }
  }
  else
  {
    return DEVICEID_INVALID;
  }
}

int
uInput::create_uinput_device(const ButtonEvent& event)
{
  if (event.is_valid())
  {
    if (event.device_id == DEVICEID_AUTO)
    {
      if (cfg.extra_devices)
      {
        if (event.type == EV_KEY)
        {
          if (is_mouse_button(event.code))
          {
            return create_uinput_device(DEVICEID_MOUSE);
          }
          else if (is_keyboard_button(event.code))
          {
            return create_uinput_device(DEVICEID_KEYBOARD);
          }
          else
          {
            return create_uinput_device(DEVICEID_JOYSTICK);
          }
        }
        else if (event.type == EV_REL)
        {
          return create_uinput_device(DEVICEID_MOUSE);
        }
        else
        {
          return create_uinput_device(DEVICEID_JOYSTICK);
        }
      }
      else
      {
        return create_uinput_device(DEVICEID_JOYSTICK);
      }
    }
    else
    {
      return create_uinput_device(event.device_id);
    }
  }
  else
  {
    return DEVICEID_INVALID;
  }
}

void
uInput::setup_xbox360_gamepad(GamepadType type)
{
  // LED
  //ioctl(fd, UI_SET_EVBIT, EV_LED);
  //ioctl(fd, UI_SET_LEDBIT, LED_MISC);

  if (cfg.force_feedback)
  {
    // 
    get_force_feedback_uinput()->add_ff(FF_RUMBLE);
    get_force_feedback_uinput()->add_ff(FF_PERIODIC);
    get_force_feedback_uinput()->add_ff(FF_CONSTANT);
    get_force_feedback_uinput()->add_ff(FF_RAMP);

    // Periodic effect subtypes
    get_force_feedback_uinput()->add_ff(FF_SINE);
    get_force_feedback_uinput()->add_ff(FF_TRIANGLE);
    get_force_feedback_uinput()->add_ff(FF_SQUARE);
    get_force_feedback_uinput()->add_ff(FF_SAW_UP);
    get_force_feedback_uinput()->add_ff(FF_SAW_DOWN);
    get_force_feedback_uinput()->add_ff(FF_CUSTOM);

    // Gain support
    get_force_feedback_uinput()->add_ff(FF_GAIN);

    // Unsupported effects
    // get_force_feedback_uinput()->add_ff(FF_SPRING);
    // get_force_feedback_uinput()->add_ff(FF_FRICTION);
    // get_force_feedback_uinput()->add_ff(FF_DAMPER);
    // get_force_feedback_uinput()->add_ff(FF_INERTIA);

    // FF_GAIN     - relative strength of rumble
    // FF_RUMBLE   - basic rumble (delay, time)
    // FF_CONSTANT - envelope, emulate with rumble
    // FF_RAMP     - same as constant, except strength grows
    // FF_PERIODIC - envelope
    // |- FF_SINE      types of periodic effects
    // |- FF_TRIANGLE
    // |- FF_SQUARE
    // |- FF_SAW_UP
    // |- FF_SAW_DOWN
    // '- FF_CUSTOM
  }

  if (cfg.dpad_only)
  {
    add_axis(XBOX_AXIS_X1, -1, 1);
    add_axis(XBOX_AXIS_Y1, -1, 1);
  }
  else
  {
    add_axis(XBOX_AXIS_X1, -32768, 32767);
    add_axis(XBOX_AXIS_Y1, -32768, 32767);
  }

  if (!cfg.dpad_only)
  {  
    add_axis(XBOX_AXIS_X2, -32768, 32767);
    add_axis(XBOX_AXIS_Y2, -32768, 32767);
  }

  if (cfg.trigger_as_button)
  {
    add_button(XBOX_BTN_LT);
    add_button(XBOX_BTN_RT);
  }
  else if (cfg.trigger_as_zaxis)
  {
    add_axis(XBOX_AXIS_TRIGGER, -255, 255);
  }
  else
  {
    add_axis(XBOX_AXIS_LT, 0, 255);
    add_axis(XBOX_AXIS_RT, 0, 255);
  }

  if (!cfg.dpad_only)
  {
    if (!cfg.dpad_as_button)
    {
      add_axis(XBOX_AXIS_DPAD_X, -1, 1);
      add_axis(XBOX_AXIS_DPAD_Y, -1, 1);
    }
    else
    {
      add_button(XBOX_DPAD_UP);
      add_button(XBOX_DPAD_DOWN);
      add_button(XBOX_DPAD_LEFT);
      add_button(XBOX_DPAD_RIGHT);
    }
  }

  add_button(XBOX_BTN_START);
  add_button(XBOX_BTN_BACK);
        
  if (type == GAMEPAD_XBOX360 || 
      type == GAMEPAD_XBOX360_WIRELESS)
    add_button(XBOX_BTN_GUIDE);

  add_button(XBOX_BTN_A);
  add_button(XBOX_BTN_B);
  add_button(XBOX_BTN_X);
  add_button(XBOX_BTN_Y);

  add_button(XBOX_BTN_LB);
  add_button(XBOX_BTN_RB);

  add_button(XBOX_BTN_THUMB_L);
  add_button(XBOX_BTN_THUMB_R);
}

void
uInput::setup_xbox360_guitar()
{
  // Whammy and Tilt
  add_axis(XBOX_AXIS_X1, -32768, 32767);
  add_axis(XBOX_AXIS_Y1, -32768, 32767);

  // Dpad
  add_button(XBOX_DPAD_UP);
  add_button(XBOX_DPAD_DOWN);
  add_button(XBOX_DPAD_LEFT);
  add_button(XBOX_DPAD_RIGHT);

  // Base
  add_button(XBOX_BTN_START);
  add_button(XBOX_BTN_BACK);
  add_button(XBOX_BTN_GUIDE);

  // Fret button
  add_button(XBOX_BTN_GREEN);
  add_button(XBOX_BTN_RED);
  add_button(XBOX_BTN_BLUE);
  add_button(XBOX_BTN_YELLOW);
  add_button(XBOX_BTN_ORANGE);
}

uInput::~uInput()
{
}

void
uInput::send(XboxGenericMsg& msg)
{
  switch(msg.type)
  {
    case XBOX_MSG_XBOX:
      send(msg.xbox);
      break;

    case XBOX_MSG_XBOX360:
      send(msg.xbox360);
      break;

    case XBOX_MSG_XBOX360_GUITAR:
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

  if (!cfg.dpad_only)
  {
    send_axis(XBOX_AXIS_X1,  msg.x1);
    send_axis(XBOX_AXIS_Y1, -msg.y1);

    send_axis(XBOX_AXIS_X2,  msg.x2);
    send_axis(XBOX_AXIS_Y2, -msg.y2);
  }

  if (cfg.dpad_as_button)
  {
    send_button(XBOX_DPAD_UP,    msg.dpad_up);
    send_button(XBOX_DPAD_DOWN,  msg.dpad_down);
    send_button(XBOX_DPAD_LEFT,  msg.dpad_left);
    send_button(XBOX_DPAD_RIGHT, msg.dpad_right);
  }
  else
  {
    int dpad_x = XBOX_AXIS_DPAD_X;
    int dpad_y = XBOX_AXIS_DPAD_Y;
      
    if (cfg.dpad_only)
    {
      dpad_x = XBOX_AXIS_X1;
      dpad_y = XBOX_AXIS_Y1;
    }

    if      (msg.dpad_up)    send_axis(dpad_y, -1);
    else if (msg.dpad_down)  send_axis(dpad_y,  1);
    else                     send_axis(dpad_y,  0);

    if      (msg.dpad_left)  send_axis(dpad_x, -1);
    else if (msg.dpad_right) send_axis(dpad_x,  1);
    else                     send_axis(dpad_x,  0);
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


  if (!cfg.dpad_only)
  {
    send_axis(XBOX_AXIS_X1,  msg.x1);
    send_axis(XBOX_AXIS_Y1, -msg.y1);

    send_axis(XBOX_AXIS_X2,  msg.x2);
    send_axis(XBOX_AXIS_Y2, -msg.y2);
  }

  if (cfg.dpad_as_button)
  {
    send_button(XBOX_DPAD_UP,    msg.dpad_up);
    send_button(XBOX_DPAD_DOWN,  msg.dpad_down);
    send_button(XBOX_DPAD_LEFT,  msg.dpad_left);
    send_button(XBOX_DPAD_RIGHT, msg.dpad_right);
  }
  else
  {
    int dpad_x = XBOX_AXIS_DPAD_X;
    int dpad_y = XBOX_AXIS_DPAD_Y;
      
    if (cfg.dpad_only)
    {
      dpad_x = XBOX_AXIS_X1;
      dpad_y = XBOX_AXIS_Y1;
    }

    if      (msg.dpad_up)    send_axis(dpad_y, -1);
    else if (msg.dpad_down)  send_axis(dpad_y,  1);
    else                     send_axis(dpad_y,  0);

    if      (msg.dpad_left)  send_axis(dpad_x, -1);
    else if (msg.dpad_right) send_axis(dpad_x,  1);
    else                     send_axis(dpad_x,  0);
  }
}

void
uInput::send(Xbox360GuitarMsg& msg)
{
  send_button(XBOX_DPAD_UP,    msg.dpad_up);
  send_button(XBOX_DPAD_DOWN,  msg.dpad_down);
  send_button(XBOX_DPAD_LEFT,  msg.dpad_left);
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

void
uInput::update(int msec_delta)
{
  // Relative Motion emulation for axis
  for(std::vector<RelAxisState>::iterator i = rel_axis.begin(); i != rel_axis.end(); ++i)
  {
    i->time += msec_delta;

    if (i->time >= i->next_time)
    {
      if (i->axis == XBOX_AXIS_TRIGGER)
      { // dirty little hack, as we can't get the axis range easily by other means
        get_mouse_uinput()->send(EV_REL, cfg.axis_map[i->axis].code,
                                 static_cast<int>(cfg.axis_map[i->axis].rel.value * axis_state[i->axis]) / 255);
      }
      else
      {
        get_mouse_uinput()->send(EV_REL, cfg.axis_map[i->axis].code,
                                 static_cast<int>(cfg.axis_map[i->axis].rel.value * axis_state[i->axis]) / 32767);
      }
      i->next_time += cfg.axis_map[i->axis].rel.repeat;
    }
  }

  // Relative Motion emulation for button
  for(std::vector<RelButtonState>::iterator i = rel_button.begin(); i != rel_button.end(); ++i)
  {
    i->time += msec_delta;

    if (i->time >= i->next_time)
    {
      get_mouse_uinput()->send(EV_REL, cfg.btn_map.lookup(i->button).code, 
                               static_cast<int>(cfg.btn_map.lookup(i->button).rel.value * button_state[i->button]));
      i->next_time += cfg.btn_map.lookup(i->button).rel.repeat;
    }
  }

  get_force_feedback_uinput()->update_force_feedback(msec_delta);

  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->sync();
  }
}

void
uInput::send_button(int code, bool value)
{
  if (button_state[code] != value)
  {
    button_state[code] = value;

    // in case a shift button was changed, we have to clear all
    // connected buttons
    for(int i = 0; i < XBOX_BTN_MAX; ++i) // iterate over all buttons
    {
      if (button_state[i])
      {
        const ButtonEvent& event = cfg.btn_map.lookup(code, i);
        if (event.is_valid())
        {
          for(int j = 0; j < XBOX_BTN_MAX; ++j) // iterate over all shift buttons
          {
            const ButtonEvent& event = cfg.btn_map.lookup(j, i);
            send_key(event.device_id, event.code, false);
          }
        }
      }
    }

    // Shifted button events
    for(int i = 0; i < XBOX_BTN_MAX; ++i)
    {
      if (button_state[i]) // shift button is pressed
      {
        const ButtonEvent& event = cfg.btn_map.lookup(i, code);
        if (event.is_valid())
        {
          send_key(event.device_id, event.code, value);

          // exit after the first successful event, so we don't send
          // multiple events for the same button
          return;
        }
      }
    }

    // Non shifted button events
    const ButtonEvent& event = cfg.btn_map.lookup(code);
    send_key(event.device_id, event.code, value);
  }
}

void
uInput::add_key(int device_id, int ev_code)
{
  get_uinput(device_id)->add_key(ev_code);
}

void
uInput::send_key(int device_id, int ev_code, bool value)
{
  if (ev_code == -1)
  {
    // pass
  }
  else
  {
    get_uinput(device_id)->send(EV_KEY, ev_code, value);
  }
}

void
uInput::send_axis(int code, int32_t value)
{
  if (axis_state[code] != value)
  {
    int old_value = axis_state[code];
    axis_state[code] = value;

    const AxisEvent& event = cfg.axis_map[code];

    switch(event.type)
    {
      case -1:
        break;

      case EV_ABS:
        if (event.type == EV_ABS || event.type == EV_KEY)
          get_uinput(event.device_id)->send(event.type, event.code, value);
        break;

      case EV_REL:
        // Mouse events are handled in update() (which is wrong,
        // since we miss the first click and introduce a delay)
        break;

      case EV_KEY:
        if (abs(old_value) <  event.key.threshold &&
            abs(value)     >= event.key.threshold)
        { // entering bigger then threshold zone
          if (value < 0)
          {
            send_key(event.device_id, event.key.secondary_code, false);
            send_key(event.device_id, event.code,               true);
          }
          else // (value > 0)
          { 
            send_key(event.device_id, event.code,               false);
            send_key(event.device_id, event.key.secondary_code, true);
          }
        }
        else if (abs(old_value) >= event.key.threshold &&
                 abs(value)     <  event.key.threshold)
        { // entering zero zone
          send_key(event.device_id, event.code,               false);
          send_key(event.device_id, event.key.secondary_code, false);
        }
        break;
    }
  }
}

void
uInput::add_axis(int code, int min, int max)
{
  const AxisEvent& event = cfg.axis_map[code];

  switch(event.type)
  {
    case EV_ABS:
      get_uinput(event.device_id)->add_abs(event.code, min, max, event.abs.fuzz, event.abs.flat);
      break;
    
    case EV_REL:
    {
      get_uinput(event.device_id)->add_rel(event.code);

      RelAxisState rel_axis_state;
      rel_axis_state.axis = code;
      rel_axis_state.time = 0;
      rel_axis_state.next_time = 0;

      rel_axis.push_back(rel_axis_state);
    }
    break;

    case EV_KEY:
      add_key(event.device_id, event.code);
      if (event.code != event.key.secondary_code)
      {
        add_key(event.device_id, event.key.secondary_code);
      }
      break;

    case -1:
      break;

    default:
      std::cout << "uInput: Unhandled event type: " << event.type << std::endl;
      break;
  }
}

void
uInput::add_button(int code)
{
  for(int i = 0; i < XBOX_BTN_MAX; ++i)
  {
    const ButtonEvent& event = cfg.btn_map.lookup(i, code);

    if (event.type == EV_KEY)
    {
      add_key(event.device_id, event.code);
    }
    else if (event.type == EV_REL)
    {
      get_uinput(event.device_id)->add_rel(event.code);

      RelButtonState rel_button_state;
      rel_button_state.button = code;
      rel_button_state.time = 0;
      rel_button_state.next_time = 0;
      rel_button.push_back(rel_button_state);
    }
    else if (event.type == EV_ABS)
    {
    }
  }
}

LinuxUinput*
uInput::get_uinput(int device_id) const
{
  uInputDevs::const_iterator it = uinput_devs.find(device_id);
  if (it != uinput_devs.end())
  {
    return it->second.get();
  }
  else
  {
    std::ostringstream str;
    str << "Couldn't find uinput device: " << device_id;
    throw std::runtime_error(str.str());
  }
}

LinuxUinput*
uInput::get_mouse_uinput() const
{
  return get_uinput(DEVICEID_MOUSE);
}

LinuxUinput*
uInput::get_force_feedback_uinput() const
{
  return get_uinput(0);
}

void
uInput::set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback)
{
  get_force_feedback_uinput()->set_ff_callback(callback);
}

/* EOF */
