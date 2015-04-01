/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#include "namespace.hpp"

Namespace::Namespace(const std::string& name) :
  m_name(name),
  m_symbols()
{
}

SymbolPtr
Namespace::lookup(const std::string& name)
{
  Symbols::iterator it = m_symbols.find(name);
  if (it == m_symbols.end())
  {
    return SymbolPtr();
  }
  else
  {
    return it->second;
  }
}

SymbolPtr
Namespace::add_symbol(const std::string& name)
{
  log_debug(name);

  Symbols::iterator it = m_symbols.find(name);
  if (it != m_symbols.end())
  {
    raise_exception(std::runtime_error, "name conflict for symbol: " << name);
  }
  else
  {
    SymbolPtr sym(new Symbol(*this, name));
    m_symbols[name] = sym;
    return sym;
  }
}

void
Namespace::add_alias(const std::string& name, SymbolPtr sym)
{
  Symbols::iterator it = m_symbols.find(name);
  if (it != m_symbols.end())
  {
    raise_exception(std::runtime_error, "name conflict for symbol: " << name);
  }
  else
  {
    m_symbols[name] = sym;
  }
}

/* EOF */
