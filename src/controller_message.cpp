/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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
#include <ostream>
#include <string.h>

#include "util/math.hpp"
#include "util/string.hpp"
#include "controller_message_descriptor.hpp"

namespace xboxdrv {

ControllerMessage::ControllerMessage() :
  // FIXME: should get proper size from ControllerMessageDescriptor
  // instead of hardcoded defaults
  m_abs_state(),
  m_abs_min(),
  m_abs_max(),
  m_rel_state(),
  m_key_state()
{
}

void
ControllerMessage::clear()
{
  std::fill(m_abs_state.begin(), m_abs_state.end(), 0);
  std::fill(m_abs_min.begin(), m_abs_min.end(), 0);
  std::fill(m_abs_max.begin(), m_abs_max.end(), 0);
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
ControllerMessage::get_abs(int abs) const
{
  return m_abs_state[abs];
}

void
ControllerMessage::set_abs(int abs, int v, int min, int max)
{
  m_abs_state[abs] = v;
  m_abs_min[abs] = min;
  m_abs_max[abs] = max;
}

float
ControllerMessage::get_abs_float(int abs) const
{
  return to_float(m_abs_state[abs], m_abs_min[abs], m_abs_max[abs]);
}

void
ControllerMessage::set_abs_float(int abs, float v)
{
  m_abs_state[abs] = from_float(v, -32768, 32767);
  m_abs_min[abs] = -32768;
  m_abs_max[abs] = 32767;
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
ControllerMessage::get_abs_min(int abs)
{
  return m_abs_min[abs];
}

int
ControllerMessage::get_abs_max(int abs)
{
  return m_abs_max[abs];
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

std::ostream& format_generic(std::ostream& out, const ControllerMessage& msg, const ControllerMessageDescriptor& desc)
{
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

} // namespace xboxdrv

/* EOF */
