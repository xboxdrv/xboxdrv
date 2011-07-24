/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "uinput_config.hpp"

#include "controller_message.hpp"
#include "helper.hpp"
#include "uinput.hpp"
#include "uinput_options.hpp"

UInputConfig::UInputConfig(UInput& uinput, int slot, bool extra_devices, const UInputOptions& opts) :
  m_uinput(uinput),
  m_btn_map(opts.get_btn_map()), 
  m_axis_map(opts.get_axis_map()),
  m_button_state(),
  m_last_button_state()
{
  std::fill_n(axis_state,   static_cast<int>(XBOX_AXIS_MAX), 0);

  m_btn_map.init(uinput, slot, extra_devices);
  m_axis_map.init(uinput, slot, extra_devices);
}

void
UInputConfig::send(const ControllerMessage& msg)
{
  m_last_button_state = m_button_state;
 
  for(int btn = 1; btn < XBOX_BTN_MAX; ++btn)
  {
    m_button_state[btn] = msg.get_button(static_cast<XboxButton>(btn));
  }
  m_btn_map.send(m_uinput, m_button_state);

  for(int axis = 1; axis < XBOX_AXIS_MAX; ++axis)
  {
    send_axis(static_cast<XboxAxis>(axis), msg.get_axis(static_cast<XboxAxis>(axis)));
  }

  m_uinput.sync();
}

void
UInputConfig::update(int msec_delta)
{
  m_btn_map.update(m_uinput, msec_delta);
  m_axis_map.update(m_uinput, msec_delta);

  m_uinput.sync();
}

void
UInputConfig::reset_all_outputs()
{
  m_last_button_state.reset();
  m_button_state.reset();

  m_btn_map.send_clear(m_uinput);

  // FIXME: incorrect, due to the effect of filter
  for(int axis = 1; axis < XBOX_AXIS_MAX; ++axis)
  {
    send_axis(static_cast<XboxAxis>(axis), 0);
  }

  m_uinput.sync();
}

void
UInputConfig::send_axis(XboxAxis code, int32_t value)
{
  AxisEventPtr ev = m_axis_map.lookup(code);
  AxisEventPtr last_ev = ev;

  // find the curren AxisEvent bound to current axis code
  for(int shift = 1; shift < XBOX_BTN_MAX; ++shift)
  {
    if (m_button_state[shift])
    {
      AxisEventPtr new_ev = m_axis_map.lookup(static_cast<XboxButton>(shift), code);
      if (new_ev)
      {
        ev = new_ev;
        break;
      }
    }
  }
  
  // find the last AxisEvent bound to current axis code
  for(int shift = 1; shift < XBOX_BTN_MAX; ++shift)
  {    
    if (m_last_button_state[shift])
    {
      AxisEventPtr new_ev = m_axis_map.lookup(static_cast<XboxButton>(shift), code);
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
    if (last_ev) last_ev->send(m_uinput, 0);
    if (ev) ev->send(m_uinput, value);
  }
  else
  {
    // no shift was touched, so only send events when the value changed
    if (axis_state[code] != value)
    {
      if (ev) ev->send(m_uinput, value);
    }
  }

  // save current value
  axis_state[code] = value;
}

/* EOF */
