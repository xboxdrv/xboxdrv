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

uInput::uInput(GamepadType type, int vendor_id, int product_id, uInputCfg config_) :
  m_vendor_id(vendor_id),
  m_product_id(product_id),
  uinput_devs(),
  cfg(config_),
  rel_repeat_lst()
{
  std::fill_n(axis_state,   static_cast<int>(XBOX_AXIS_MAX), 0);
  std::fill_n(button_state,      static_cast<int>(XBOX_BTN_MAX),  false);
  std::fill_n(last_button_state, static_cast<int>(XBOX_BTN_MAX),  false);

  if (cfg.force_feedback)
  {
    create_uinput_device(DEVICEID_JOYSTICK);
  }

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

  for(int n = 0; n < cfg.input_mapping_count(); ++n)
  {
    cfg.get_btn_map(n).init(*this);
    cfg.get_axis_map(n).init(*this);
  }

  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->finish();
  }
}

void
uInput::create_uinput_device(int device_id)
{
  // DEVICEID_AUTO should not happen at this point as the user should
  // have called resolve_device_id()
  assert(device_id != DEVICEID_AUTO);

  uInputDevs::iterator it = uinput_devs.find(device_id);
  if (it != uinput_devs.end())
  {
    // device already exist, which is fine    
  }
  else
  {
    LinuxUinput::DeviceType device_type = LinuxUinput::kGenericDevice;
    std::ostringstream dev_name;
    dev_name << cfg.device_name;

    switch (device_id)
    {
      case DEVICEID_JOYSTICK:
        device_type = LinuxUinput::kJoystickDevice;
        break;

      case DEVICEID_MOUSE:
        device_type = LinuxUinput::kMouseDevice;
        dev_name << " - Mouse Emulation";
        break;
      
      case DEVICEID_KEYBOARD:
        device_type = LinuxUinput::kGenericDevice;
        dev_name << " - Keyboard Emulation";
        break;

      default:
        dev_name << " - " << device_id+1;
        break;
    }

    boost::shared_ptr<LinuxUinput> dev(new LinuxUinput(device_type, dev_name.str(), m_vendor_id, m_product_id));
    uinput_devs.insert(std::pair<int, boost::shared_ptr<LinuxUinput> >(device_id, dev));

    std::cout << "Creating uinput device: device_id: " << device_id << ", dev_name: " << dev_name.str() << std::endl;
  }
}

uInput::~uInput()
{
}

void
uInput::send(XboxGenericMsg& msg)
{
  std::copy(button_state, button_state+XBOX_BTN_MAX, last_button_state);

  switch(msg.type)
  {
    case XBOX_MSG_XBOX:
      send(msg.xbox);
      break;

    case XBOX_MSG_XBOX360:
      send(msg.xbox360);
      break;
        
    default:
      std::cout << "XboxGenericMsg type: " << msg.type << std::endl;
      assert(!"uInput: Unknown XboxGenericMsg type");
  }

  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->sync();
  }
}

void
uInput::send(Xbox360Msg& msg)
{
  // analog stick button
  send_button(XBOX_BTN_THUMB_L, msg.thumb_l);
  send_button(XBOX_BTN_THUMB_R, msg.thumb_r);

  // shoulder button
  send_button(XBOX_BTN_LB, msg.lb);
  send_button(XBOX_BTN_RB, msg.rb);

  // start/back button
  send_button(XBOX_BTN_START, msg.start);
  send_button(XBOX_BTN_GUIDE, msg.guide);
  send_button(XBOX_BTN_BACK, msg.back);

  // face button
  send_button(XBOX_BTN_A, msg.a);
  send_button(XBOX_BTN_B, msg.b);
  send_button(XBOX_BTN_X, msg.x);
  send_button(XBOX_BTN_Y, msg.y);

  // trigger
  send_button(XBOX_BTN_LT, msg.lt); // FIXME: no deadzone handling here
  send_button(XBOX_BTN_RT, msg.rt);

  // dpad
  send_button(XBOX_DPAD_UP,    msg.dpad_up);
  send_button(XBOX_DPAD_DOWN,  msg.dpad_down);
  send_button(XBOX_DPAD_LEFT,  msg.dpad_left);
  send_button(XBOX_DPAD_RIGHT, msg.dpad_right);

  // trigger
  send_axis(XBOX_AXIS_LT, msg.lt);
  send_axis(XBOX_AXIS_RT, msg.rt);

  send_axis(XBOX_AXIS_TRIGGER, (int(msg.rt) - int(msg.lt)));

  // analog sticks
  send_axis(XBOX_AXIS_X1,  msg.x1);
  send_axis(XBOX_AXIS_Y1, -msg.y1);
  
  send_axis(XBOX_AXIS_X2,  msg.x2);
  send_axis(XBOX_AXIS_Y2, -msg.y2);

  // dpad
  if      (msg.dpad_up)    send_axis(XBOX_AXIS_DPAD_Y, -1);
  else if (msg.dpad_down)  send_axis(XBOX_AXIS_DPAD_Y,  1);
  else                     send_axis(XBOX_AXIS_DPAD_Y,  0);

  if      (msg.dpad_left)  send_axis(XBOX_AXIS_DPAD_X, -1);
  else if (msg.dpad_right) send_axis(XBOX_AXIS_DPAD_X,  1);
  else                     send_axis(XBOX_AXIS_DPAD_X,  0);
}

void
uInput::send(XboxMsg& msg)
{
  // analog stick button
  send_button(XBOX_BTN_THUMB_L, msg.thumb_l);
  send_button(XBOX_BTN_THUMB_R, msg.thumb_r);

  // start/back button
  send_button(XBOX_BTN_START, msg.start);
  send_button(XBOX_BTN_BACK,  msg.back);

  // face button
  send_button(XBOX_BTN_A, msg.a);
  send_button(XBOX_BTN_B, msg.b);
  send_button(XBOX_BTN_X, msg.x);
  send_button(XBOX_BTN_Y, msg.y);

  send_button(XBOX_BTN_LB, msg.white);
  send_button(XBOX_BTN_RB, msg.black);

  // trigger
  send_button(XBOX_BTN_LT, msg.lt);
  send_button(XBOX_BTN_RT, msg.rt);

  // dpad as button
  send_button(XBOX_DPAD_UP,    msg.dpad_up);
  send_button(XBOX_DPAD_DOWN,  msg.dpad_down);
  send_button(XBOX_DPAD_LEFT,  msg.dpad_left);
  send_button(XBOX_DPAD_RIGHT, msg.dpad_right);

  send_axis(XBOX_AXIS_LT, msg.lt);
  send_axis(XBOX_AXIS_RT, msg.rt);

  send_axis(XBOX_AXIS_TRIGGER, (int(msg.rt) - int(msg.lt)));

  // analog sticks
  send_axis(XBOX_AXIS_X1,  msg.x1);
  send_axis(XBOX_AXIS_Y1, -msg.y1);

  send_axis(XBOX_AXIS_X2,  msg.x2);
  send_axis(XBOX_AXIS_Y2, -msg.y2);

  // dpad as axis
  if      (msg.dpad_up)    send_axis(XBOX_AXIS_DPAD_Y, -1);
  else if (msg.dpad_down)  send_axis(XBOX_AXIS_DPAD_Y,  1);
  else                     send_axis(XBOX_AXIS_DPAD_Y,  0);

  if      (msg.dpad_left)  send_axis(XBOX_AXIS_DPAD_X, -1);
  else if (msg.dpad_right) send_axis(XBOX_AXIS_DPAD_X,  1);
  else                     send_axis(XBOX_AXIS_DPAD_X,  0);

  send_axis(XBOX_AXIS_A, msg.a);
  send_axis(XBOX_AXIS_B, msg.b);
  send_axis(XBOX_AXIS_X, msg.x);
  send_axis(XBOX_AXIS_Y, msg.y);
  send_axis(XBOX_AXIS_BLACK, msg.black);
  send_axis(XBOX_AXIS_WHITE, msg.white);
}

void
uInput::update(int msec_delta)
{
  cfg.get_btn_map().update(*this, msec_delta);
  cfg.get_axis_map().update(*this, msec_delta);

  for(std::map<UIEvent, RelRepeat>::iterator i = rel_repeat_lst.begin(); i != rel_repeat_lst.end(); ++i)
  {
    i->second.time_count += msec_delta;

    while (i->second.time_count >= i->second.repeat_interval)
    {
      get_uinput(i->second.code.device_id)->send(EV_REL, i->second.code.code, i->second.value);
      i->second.time_count -= i->second.repeat_interval;
    }
  }

  if (cfg.force_feedback)
  {
    get_force_feedback_uinput()->update_force_feedback(msec_delta);
  }

  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->sync();
  }
}

void
uInput::send_button(XboxButton code, bool value)
{
  if (button_state[code] != value)
  {
    button_state[code] = value;

    if (code == cfg.config_toggle_button)
    {
      if (value)
      {
        reset_all_outputs();
        cfg.next_input_mapping();
      }
    }
    else
    {
      // in case a shift button was changed, we have to clear all
      // connected buttons
      for(int i = 0; i < XBOX_BTN_MAX; ++i) // iterate over all buttons
      {
        if (button_state[i])
        {
          const ButtonEventPtr& event = cfg.get_btn_map().lookup(code, static_cast<XboxButton>(i));
          if (event)
          {
            for(int j = 0; j < XBOX_BTN_MAX; ++j) // iterate over all shift buttons
            {
              cfg.get_btn_map().send(*this, 
                                     static_cast<XboxButton>(j), static_cast<XboxButton>(i), 
                                     false);
            }
          }
        }
      }

      // Shifted button events
      for(int i = 0; i < XBOX_BTN_MAX; ++i)
      {
        if (button_state[i]) // shift button is pressed
        {
          if (cfg.get_btn_map().send(*this, static_cast<XboxButton>(i), code, value))
          {
            // exit after the first successful event, so we don't send
            // multiple events for the same button
            return;
          }
        }
      }

      // Non shifted button events
      cfg.get_btn_map().send(*this, code, value);
    }
  }
}

void
uInput::reset_all_outputs()
{
  // FIXME: kind of a hack
  Xbox360Msg msg;
  memset(&msg, 0, sizeof(msg));
  send(msg);
}

void
uInput::add_key(int device_id, int ev_code)
{
  get_uinput(device_id)->add_key(ev_code);
}

void
uInput::add_rel(int device_id, int ev_code)
{
  get_uinput(device_id)->add_rel(ev_code);
}

void
uInput::add_abs(int device_id, int ev_code, int min, int max, int fuzz, int flat)
{
  get_uinput(device_id)->add_abs(ev_code, min, max, fuzz, flat);
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
uInput::sync()
{
  for(uInputDevs::iterator i = uinput_devs.begin(); i != uinput_devs.end(); ++i)
  {
    i->second->sync();
  }
}

void
uInput::send_rel_repetitive(const UIEvent& code, int value, int repeat_interval)
{
  if (repeat_interval < 0)
  { // remove rel_repeats from list
    rel_repeat_lst.erase(code);
    // no need to send a event for rel, as it defaults to 0 anyway
  }
  else
  { // add rel_repeats to list
    std::map<UIEvent, RelRepeat>::iterator it = rel_repeat_lst.find(code);

    if (it == rel_repeat_lst.end())
    {
      RelRepeat rel_rep;
      rel_rep.code  = code;
      rel_rep.value = value;
      rel_rep.time_count = 0;
      rel_rep.repeat_interval = repeat_interval;
      rel_repeat_lst.insert(std::pair<UIEvent, RelRepeat>(code, rel_rep));
    
      // Send the event once
      get_uinput(code.device_id)->send(EV_REL, code.code, value);
    }
    else
    {
      it->second.code  = code;
      it->second.value = value;
      // it->second.time_count = do not touch this
      it->second.repeat_interval = repeat_interval;
    }
  }
}

void
uInput::send_axis(XboxAxis code, int32_t value)
{
  AxisEventPtr ev = cfg.get_axis_map().lookup(code);
  AxisEventPtr last_ev = ev;

  // find the curren AxisEvent bound to current axis code
  for(int shift = 1; shift < XBOX_BTN_MAX; ++shift)
  {
    if (button_state[shift])
    {
      AxisEventPtr new_ev = cfg.get_axis_map().lookup(static_cast<XboxButton>(shift), code);
      if (new_ev)
      {
        ev = new_ev;
      }
      break;
    }
  }
  
  // find the last AxisEvent bound to current axis code
  for(int shift = 1; shift < XBOX_BTN_MAX; ++shift)
  {    
    if (last_button_state[shift])
    {
      AxisEventPtr new_ev = cfg.get_axis_map().lookup(static_cast<XboxButton>(shift), code);
      if (new_ev)
      {
        last_ev = new_ev;
      }
      break;
    }
  }

  if (last_ev != ev)
  {
    // a shift key was released
    if (last_ev) last_ev->send(*this, 0);
    if (ev) ev->send(*this, value);
  }
  else
  {
    // no shift was touched, so only send events when the value changed
    if (axis_state[code] != value)
    {
      if (ev) ev->send(*this, value);
    }
  }

  // save current value
  axis_state[code] = value;
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
    assert(0);
    std::ostringstream str;
    str << "Couldn't find uinput device: " << device_id;
    throw std::runtime_error(str.str());
  }
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
