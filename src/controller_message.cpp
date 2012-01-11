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

ControllerMessage::ControllerMessage() :
  m_axis_state(255), // BROKEN: need to get proper size from ControllerMessageDescriptor
  m_rel_state(255),
  m_button_state(255)
{
}

ControllerMessage::ControllerMessage(int num_key, int num_axis, int num_rel) :
  m_axis_state(num_axis),
  m_rel_state(num_rel),
  m_button_state(num_key)
{
}

void
ControllerMessage::clear()
{
  std::fill(m_axis_state.begin(), m_axis_state.end(), 0);
  m_button_state.reset();
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
  return m_button_state[button];
}

void
ControllerMessage::set_button(XboxButton button, bool v)
{
  m_button_state[button] = v;
}

int
ControllerMessage::get_axis(XboxAxis axis) const
{
  return m_axis_state[axis];
}

void
ControllerMessage::set_axis(XboxAxis axis, int v)
{
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
  m_axis_state[axis] = from_float(v, get_axis_min(axis), get_axis_max(axis));
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

std::ostream& format_playstation3(std::ostream& out, const ControllerMessage& msg)
{
  out << boost::format("X1:%3d Y1:%3d")
    % int(msg.get_axis(XBOX_AXIS_X1)) % int(msg.get_axis(XBOX_AXIS_Y1));

  out << boost::format("  X2:%3d Y2:%3d")
    % int(msg.get_axis(XBOX_AXIS_X2)) % int(msg.get_axis(XBOX_AXIS_Y2));
#if 0
  // FIXME: analog data gets currently not recorded in the message
  out << boost::format("  du:%3d dd:%3d dl:%3d dr:%3d")
    % int(msg.a_dpad_up)
    % int(msg.a_dpad_down)
    % int(msg.a_dpad_left)
    % int(msg.a_dpad_right);

  out << "  select:" << msg.select;
  out << " ps:" << msg.playstation;
  out << " start:" << msg.start;

  out << boost::format("  L3:%d R3:%d") % static_cast<int>(msg.l3) % static_cast<int>(msg.r3);

  out << boost::format("  /\\:%3d O:%3d X:%3d []:%3d  L1:%3d R1:%3d")
    % static_cast<int>(msg.a_triangle)
    % static_cast<int>(msg.a_circle)
    % static_cast<int>(msg.a_cross)
    % static_cast<int>(msg.a_square)
    % static_cast<int>(msg.a_l1)
    % static_cast<int>(msg.a_r1);

  out << boost::format("  L2:%3d R2:%3d")
    % int(msg.a_l2) % int(msg.a_r2);
#endif
  return out;
}

std::ostream& format_xbox360(std::ostream& out, const ControllerMessage& msg)
{
  out << boost::format("X1:%6d Y1:%6d") 
    % int(msg.get_axis(XBOX_AXIS_X1)) % int(msg.get_axis(XBOX_AXIS_Y1));

  out << boost::format("  X2:%6d Y2:%6d")
    % int(msg.get_axis(XBOX_AXIS_X2)) % int(msg.get_axis(XBOX_AXIS_Y2));
                          
  out << boost::format("  du:%d dd:%d dl:%d dr:%d")
    % int(msg.get_button(XBOX_DPAD_UP))
    % int(msg.get_button(XBOX_DPAD_DOWN))
    % int(msg.get_button(XBOX_DPAD_LEFT))
    % int(msg.get_button(XBOX_DPAD_RIGHT));

  out << "  back:" << msg.get_button(XBOX_BTN_BACK);
  out << " guide:" << msg.get_button(XBOX_BTN_GUIDE);
  out << " start:" << msg.get_button(XBOX_BTN_START);

  out << "  TL:" << msg.get_button(XBOX_BTN_THUMB_L);
  out << " TR:"  << msg.get_button(XBOX_BTN_THUMB_R);

  out << "  A:" << msg.get_button(XBOX_BTN_A);
  out << " B:"  << msg.get_button(XBOX_BTN_B);
  out << " X:"  << msg.get_button(XBOX_BTN_X);
  out << " Y:"  << msg.get_button(XBOX_BTN_Y);
  
  out << "  LB:" << msg.get_button(XBOX_BTN_LB);
  out << " RB:" <<  msg.get_button(XBOX_BTN_RB);

  out << boost::format("  LT:%3d RT:%3d")
    % int(msg.get_button(XBOX_BTN_LT)) % int(msg.get_button(XBOX_BTN_RT));

  // out << " Dummy: " << msg.dummy1 << " " << msg.dummy2 << " " << msg.dummy3;

  return out;
}

std::ostream& format_xbox(std::ostream& out, const ControllerMessage& msg) 
{
  out << boost::format(" X1:%6d Y1:%6d  X2:%6d Y2:%6d "
                       " du:%d dd:%d dl:%d dr:%d "
                       " start:%d back:%d "
                       " TL:%d TR:%d "
                       " A:%3d B:%3d X:%3d Y:%3d "
                       " black:%3d white:%3d "
                       " LT:%3d RT:%3d ")
    % int(msg.get_axis(XBOX_AXIS_X1)) % int(msg.get_axis(XBOX_AXIS_Y1))
    % int(msg.get_axis(XBOX_AXIS_X2)) % int(msg.get_axis(XBOX_AXIS_Y2))

    % int(msg.get_button(XBOX_DPAD_UP))
    % int(msg.get_button(XBOX_DPAD_DOWN))
    % int(msg.get_button(XBOX_DPAD_LEFT))
    % int(msg.get_button(XBOX_DPAD_RIGHT))

    % int(msg.get_button(XBOX_BTN_START))
    % int(msg.get_button(XBOX_BTN_BACK))

    % int(msg.get_button(XBOX_BTN_THUMB_L))
    % int(msg.get_button(XBOX_BTN_THUMB_R))

    % int(msg.get_axis(XBOX_AXIS_A))
    % int(msg.get_axis(XBOX_AXIS_B))
    % int(msg.get_axis(XBOX_AXIS_X))
    % int(msg.get_axis(XBOX_AXIS_Y))

    % int(msg.get_axis(XBOX_AXIS_BLACK))
    % int(msg.get_axis(XBOX_AXIS_WHITE))

    % int(msg.get_axis(XBOX_AXIS_LT))
    % int(msg.get_axis(XBOX_AXIS_RT));

  // out << " Dummy: " << msg.dummy;

  return out;
}

/* EOF */
