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

#ifndef HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP
#define HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP

#include <map>
#include <string>

#include "symbol_table.hpp"
#include "symbols/name.hpp"

class ControllerMessageDescriptor
{
private:
  SymbolTable<AbsName> m_abs;
  SymbolTable<KeyName> m_key;
  SymbolTable<RelName> m_rel;

public:
  ControllerMessageDescriptor();

  SymbolTable<AbsName>& abs() { return m_abs; }
  SymbolTable<KeyName>& key() { return m_key; }
  SymbolTable<RelName>& rel() { return m_rel; }

  const SymbolTable<AbsName>& abs() const { return m_abs; }
  const SymbolTable<KeyName>& key() const { return m_key; }
  const SymbolTable<RelName>& rel() const { return m_rel; }

  int get_key_count() const { return m_key.size(); }
  int get_abs_count() const { return m_abs.size(); }
  int get_rel_count() const { return m_rel.size(); }
};

#endif

/* EOF */
