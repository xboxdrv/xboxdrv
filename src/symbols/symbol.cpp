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
#include "symbol.hpp"

Symbol::Symbol(Namespace& ns, const std::string& name) :
  m_namespace(ns),
  m_name(name)
{}

void
Symbol::add_provides(SymbolPtr sym)
{
  // FIXME: implement me
}

std::string
Symbol::get_namespace() const 
{
  return m_namespace.get_name(); 
}

std::string
Symbol::get_name() const 
{
  return m_name; 
}

std::string
Symbol::str() const 
{
  std::ostringstream out;
  out << get_namespace() << "." << get_name();
  return out.str();
}

/* EOF */
