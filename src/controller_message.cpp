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

#include "controller_message.hpp"

#include <string.h>
#include <assert.h>
#include <algorithm>
#include <ostream>

#include "helper.hpp"

ControllerMessage::ControllerMessage() :
  m_axis_state(),
  m_button_state(),
  m_axis_set(),
  m_button_set()
{
  clear();
}

void
ControllerMessage::clear()
{
  std::fill_n(m_axis_state,   static_cast<int>(XBOX_AXIS_MAX), 0);
  m_button_state.reset();

  m_axis_set.reset();
  m_button_set.reset();
}

bool
ControllerMessage::get_key(int key) const
{
  return get_button(static_cast<XboxButton>(key));
}

void
ControllerMessage::set_key(int key, bool v)
{
  return set_button(static_cast<XboxButton>(key), v);
}

bool
ControllerMessage::get_button(XboxButton button) const
{
  if (m_button_set[button])
  {
    return m_button_state[button];
  }
  else
  {
    switch(button)
    {
      case XBOX_BTN_A:
        return m_axis_state[XBOX_AXIS_A];
      case XBOX_BTN_B:
        return m_axis_state[XBOX_AXIS_B];
      case XBOX_BTN_X:
        return m_axis_state[XBOX_AXIS_X];
      case XBOX_BTN_Y:
        return m_axis_state[XBOX_AXIS_Y];

      case XBOX_BTN_LB:
        return m_axis_state[XBOX_AXIS_BLACK];
      case XBOX_BTN_RB:
        return m_axis_state[XBOX_AXIS_WHITE];

      case XBOX_BTN_LT:
        return m_axis_state[XBOX_AXIS_LT];
      case XBOX_BTN_RT:
        return m_axis_state[XBOX_AXIS_RT];

      case XBOX_DPAD_UP:
        return m_axis_state[XBOX_AXIS_DPAD_Y] < 0;
      case XBOX_DPAD_DOWN:
        return m_axis_state[XBOX_AXIS_DPAD_Y] > 0;       
      case XBOX_DPAD_LEFT:
        return m_axis_state[XBOX_AXIS_DPAD_X] < 0;
      case XBOX_DPAD_RIGHT:
        return m_axis_state[XBOX_AXIS_DPAD_X] > 0;

      default:
        return false;
    }
  }
}

void
ControllerMessage::set_button(XboxButton button, bool v)
{
  m_button_set[button] = true;
  m_button_state[button] = v;
}

int
ControllerMessage::get_axis(XboxAxis axis) const
{
  if (m_axis_set[axis])
  {
    return m_axis_state[axis];
  }
  else
  {
    switch(axis)
    {
      case XBOX_AXIS_A:
        return m_button_state[XBOX_BTN_A];

      case XBOX_AXIS_B:
        return m_button_state[XBOX_BTN_B];

      case XBOX_AXIS_X:
        return m_button_state[XBOX_BTN_X];

      case XBOX_AXIS_Y:
        return m_button_state[XBOX_BTN_Y];

      case XBOX_AXIS_LT:
        return m_button_state[XBOX_BTN_LT] * 255;

      case XBOX_AXIS_RT:
        return m_button_state[XBOX_BTN_RT] * 255;

      case XBOX_AXIS_BLACK:
        return m_button_state[XBOX_BTN_LB] * 255;

      case XBOX_AXIS_WHITE:
        return m_button_state[XBOX_BTN_RB] * 255;

      case XBOX_AXIS_DPAD_X:
        if (m_button_state[XBOX_DPAD_LEFT] && !m_button_state[XBOX_DPAD_RIGHT])
          return -1;
        else if (!m_button_state[XBOX_DPAD_LEFT] && m_button_state[XBOX_DPAD_RIGHT])
          return 1;
        else
          return 0;

      case XBOX_AXIS_DPAD_Y:
        if (m_button_state[XBOX_DPAD_UP] && !m_button_state[XBOX_DPAD_DOWN])
          return -1;
        else if (!m_button_state[XBOX_DPAD_UP] && m_button_state[XBOX_DPAD_DOWN])
          return 1;
        else
          return 0;

      case XBOX_AXIS_TRIGGER:
        return -get_axis(XBOX_AXIS_LT) + get_axis(XBOX_AXIS_RT);

      default:
        return 0;
    }
  }
}

void
ControllerMessage::set_axis(XboxAxis axis, int v)
{
  m_axis_set[axis] = true;
  m_axis_state[axis] = v;
}

float
ControllerMessage::get_axis_float(XboxAxis axis) const
{
  return to_float(m_axis_state[axis], get_axis_min(axis), get_axis_max(axis));
}

void
ControllerMessage::set_axis_float(XboxAxis axis, float v)
{
  m_axis_set[axis] = true;
  m_axis_state[axis] = from_float(v, get_axis_min(axis), get_axis_max(axis));
}

bool
ControllerMessage::axis_is_set(XboxAxis axis) const
{
  return m_axis_set[axis];
}

bool
ControllerMessage::button_is_set(XboxButton button) const
{
  return m_button_set[button];
}

int
ControllerMessage::get_axis_min(XboxAxis axis)
{
  switch(axis)
  {
    case XBOX_AXIS_X1: return -32768;
    case XBOX_AXIS_Y1: return -32768;
    
    case XBOX_AXIS_X2: return -32768;
    case XBOX_AXIS_Y2: return -32768;

    case XBOX_AXIS_LT: return 0;
    case XBOX_AXIS_RT: return 0;

    case XBOX_AXIS_DPAD_X: return -1;
    case XBOX_AXIS_DPAD_Y: return -1;

    case XBOX_AXIS_TRIGGER: return -255;

    case XBOX_AXIS_A:     return 0;
    case XBOX_AXIS_B:     return 0;
    case XBOX_AXIS_X:     return 0;
    case XBOX_AXIS_Y:     return 0;
    case XBOX_AXIS_BLACK: return 0;
    case XBOX_AXIS_WHITE: return 0;

    case WIIMOTE_ACC_X: return 0;
    case WIIMOTE_ACC_Y: return 0;
    case WIIMOTE_ACC_Z: return 0;

    case NUNCHUK_ACC_X: return 0;
    case NUNCHUK_ACC_Y: return 0;
    case NUNCHUK_ACC_Z: return 0;

    case WIIMOTE_IR_X: return 0;
    case WIIMOTE_IR_Y: return 0;
    case WIIMOTE_IR_SIZE: return -128;

    case WIIMOTE_IR_X2: return 0;
    case WIIMOTE_IR_Y2: return 0;
    case WIIMOTE_IR_SIZE2: return -128;

    case WIIMOTE_IR_X3: return 0;
    case WIIMOTE_IR_Y3: return 0;
    case WIIMOTE_IR_SIZE3: return -128;

    case WIIMOTE_IR_X4: return 0;
    case WIIMOTE_IR_Y4: return 0;
    case WIIMOTE_IR_SIZE4: return -128;

    case XBOX_AXIS_UNKNOWN: return 0;
    case XBOX_AXIS_MAX: return 0;
  }

  assert(!"never reached");
}

int
ControllerMessage::get_axis_max(XboxAxis axis)
{
  switch(axis)
  {
    case XBOX_AXIS_X1: return 32767;
    case XBOX_AXIS_Y1: return 32767;
    
    case XBOX_AXIS_X2: return 32767;
    case XBOX_AXIS_Y2: return 32767;

    case XBOX_AXIS_LT: return 255;
    case XBOX_AXIS_RT: return 255;

    case XBOX_AXIS_DPAD_X: return 1;
    case XBOX_AXIS_DPAD_Y: return 1;

    case XBOX_AXIS_TRIGGER: return 255;

    case XBOX_AXIS_A:     return 255;
    case XBOX_AXIS_B:     return 255;
    case XBOX_AXIS_X:     return 255;
    case XBOX_AXIS_Y:     return 255;
    case XBOX_AXIS_BLACK: return 255;
    case XBOX_AXIS_WHITE: return 255;

    case WIIMOTE_ACC_X: return 255;
    case WIIMOTE_ACC_Y: return 255;
    case WIIMOTE_ACC_Z: return 255;

    case NUNCHUK_ACC_X: return 255;
    case NUNCHUK_ACC_Y: return 255;
    case NUNCHUK_ACC_Z: return 255;  

    case WIIMOTE_IR_X: return 1024;
    case WIIMOTE_IR_Y: return 768;
    case WIIMOTE_IR_SIZE: return 127;

    case WIIMOTE_IR_X2: return 1024;
    case WIIMOTE_IR_Y2: return 768;
    case WIIMOTE_IR_SIZE2: return 127;

    case WIIMOTE_IR_X3: return 1024;
    case WIIMOTE_IR_Y3: return 768;
    case WIIMOTE_IR_SIZE3: return 127;

    case WIIMOTE_IR_X4: return 1024;
    case WIIMOTE_IR_Y4: return 768;
    case WIIMOTE_IR_SIZE4: return 127;

    case XBOX_AXIS_UNKNOWN: return 0;
    case XBOX_AXIS_MAX: return 0;
  }
  
  assert(!"never reached");
}

void
ControllerMessage::set_axis_min(XboxAxis axis, int value)
{
}

void
ControllerMessage::set_axis_max(XboxAxis axis, int value)
{
}

std::ostream& operator<<(std::ostream& out, const ControllerMessage& msg)
{
  for(int i = 1; i < XBOX_AXIS_MAX; ++i)
  {
    XboxAxis axis = static_cast<XboxAxis>(i);

    out << axis2string(axis) << ":" << msg.get_axis(static_cast<XboxAxis>(axis)) << " ";
  }

  for(int i = 1; i < XBOX_BTN_MAX; ++i)
  {
    XboxButton btn = static_cast<XboxButton>(i);

    out << btn2string(btn) << ":" << msg.get_button(btn) << " ";
  }
  
  return out;
}

/* EOF */
