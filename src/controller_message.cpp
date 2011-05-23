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

ControllerMessage::ControllerMessage() :
  m_button(),
  m_axis(),
  m_rel()
{
  clear();
}

void
ControllerMessage::set_button(int button, bool value)
{
  assert(button >= 0);
  assert(button < static_cast<int>(sizeof(m_button)));

  m_button[button] = value;
}

int
ControllerMessage::get_button(int button) const
{
  assert(button >= 0);
  assert(button < static_cast<int>(sizeof(m_button)));

  return m_button[button];
}

void
ControllerMessage::set_axis(int axis, int value)
{
  assert(axis >= 0);
  assert(axis < static_cast<int>(sizeof(m_axis)));
  
  m_axis[axis] = value;
}

int
ControllerMessage::get_axis(int axis) const
{
  assert(axis >= 0);
  assert(axis < static_cast<int>(sizeof(m_axis)));
  
  return m_axis[axis];
}

void
ControllerMessage::set_rel(int rel, int value)
{
  assert(rel >= 0);
  assert(rel < static_cast<int>(sizeof(m_rel)));
  
  m_rel[rel] = value;
}

int
ControllerMessage::get_rel(int rel) const
{
  assert(rel >= 0);
  assert(rel < static_cast<int>(sizeof(m_rel)));
  
  return m_rel[rel];
}

void
ControllerMessage::clear()
{
  memset(m_button, 0, sizeof(m_button));
  memset(m_axis,   0, sizeof(m_axis));
  memset(m_rel,    0, sizeof(m_rel));
}

/* EOF */
