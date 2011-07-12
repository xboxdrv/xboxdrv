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

#include "ui_event_emitter.hpp"

#include <assert.h>
#include "uinput.hpp"

UIEventEmitter::UIEventEmitter(UInput& uinput, 
                               uint32_t device_id, 
                               int type, 
                               int code) :
  m_uinput(uinput),
  m_device_id(device_id),
  m_type(type),
  m_code(code)
{
  assert(m_code != -1);
}

void
UIEventEmitter::send(int value)
{
  m_uinput.send(m_device_id, m_type, m_code, value);
}

/* EOF */
