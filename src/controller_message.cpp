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

#include <algorithm>
#include <assert.h>
#include <boost/format.hpp>
#include <ostream>
#include <string.h>

#include "helper.hpp"
#include "controller_message_descriptor.hpp"

ControllerMessage::ControllerMessage() :
  // FIXME: should get proper size from ControllerMessageDescriptor
  // instead of hardcoded defaults
  m_abs_state(),
  m_rel_state(),
  m_key_state()
{
}

void
ControllerMessage::clear()
{
  std::fill(m_abs_state.begin(), m_abs_state.end(), 0);
  std::fill(m_rel_state.begin(), m_rel_state.end(), 0);
  m_key_state.reset();
}

bool
ControllerMessage::get_key(int key) const
{
  return m_key_state[key];
}

void
ControllerMessage::set_key(int key, bool v)
{
  m_key_state[key] = v;
}

int
ControllerMessage::get_abs(int axis) const
{
  return m_abs_state[axis];
}

void
ControllerMessage::set_abs(int abs, int v)
{
  m_abs_state[abs] = v;
}

float
ControllerMessage::get_abs_float(int axis) const
{
  return to_float(m_abs_state[axis], get_abs_min(axis), get_abs_max(axis));
}

void
ControllerMessage::set_abs_float(int axis, float v)
{
  m_abs_state[axis] = from_float(v, get_abs_min(axis), get_abs_max(axis));
}

int
ControllerMessage::get_rel(int rel) const
{
  return m_rel_state[rel];
}

void
ControllerMessage::set_rel(int rel, int v)
{
  m_rel_state[rel] = v;
}

int
ControllerMessage::get_abs_min(int axis)
{
#if 0
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
#endif

  return -32768; // BROKEN
}

int
ControllerMessage::get_abs_max(int abs)
{
#if 0
  switch(abs)
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
#endif  
  return 32767; // BROKEN
}

std::ostream& format_generic(std::ostream& out, const ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
  out << "generic: ";
  out << desc.get_key_count() << " ";
  for(int i = 0; i < desc.get_key_count(); ++i)
  {
    out << desc.key().get(i) << ":" << msg.get_key(i) << " ";
  }

  for(int i = 0; i < desc.get_abs_count(); ++i)
  {
    out << desc.abs().get(i) << ":" << msg.get_abs(i) << " ";
  }

  for(int i = 0; i < desc.get_rel_count(); ++i)
  {
    if (msg.get_rel(i))
    {
      out << i << " ";
    }
  }

  return out;
}

bool
ControllerMessage::operator==(const ControllerMessage& rhs) const
{
  return 
    m_abs_state == rhs.m_abs_state &&
    m_rel_state == rhs.m_rel_state &&
    m_key_state == rhs.m_key_state;
}

bool
ControllerMessage::operator!=(const ControllerMessage& rhs) const
{
  return !((*this) == rhs);
}

/* EOF */
