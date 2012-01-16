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

#ifndef HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP
#define HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP

#include <map>
#include <string>

#include "symbol_table.hpp"

class ControllerMessageDescriptor
{
private:
  SymbolTable m_abs;
  SymbolTable m_key;
  SymbolTable m_rel;

public:
  ControllerMessageDescriptor();

  SymbolTable& abs() { return m_abs; }
  SymbolTable& key() { return m_key; }
  SymbolTable& rel() { return m_rel; }

  const SymbolTable& abs() const { return m_abs; }
  const SymbolTable& key() const { return m_key; }
  const SymbolTable& rel() const { return m_rel; }

  int get_key_count() const { return m_key.size(); }
  int get_abs_count() const { return m_abs.size(); }
  int get_rel_count() const { return m_rel.size(); }
};

#endif

/* EOF */
