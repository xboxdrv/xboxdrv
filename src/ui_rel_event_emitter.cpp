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

#include "ui_rel_event_emitter.hpp"

UIRelEventEmitter::UIRelEventEmitter() : 
  m_value()
{
}

void
UIRelEventEmitter::send(int value)
{
  m_value = value;
}

int
UIRelEventEmitter::get_value() const
{
  return m_value;
}

void
UIRelEventEmitter::clear()
{
  m_value = 0;
}

/* EOF */
