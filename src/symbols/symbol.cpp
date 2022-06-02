/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

namespace xboxdrv {

Symbol::Symbol(Namespace& ns, std::string const& name) :
  m_namespace(ns),
  m_name(name),
  m_provides()
{}

bool
Symbol::match(SymbolPtr sym) const
{
  if (this == sym.get())
  {
    return true;
  }
  else
  {
    for(std::vector<Symbol*>::const_iterator it = m_provides.begin();
        it != m_provides.end();
        ++it)
    {
      if (*it == sym.get())
      {
        return true;
      }
    }

    return false;
  }
}

void
Symbol::add_provides(SymbolPtr sym)
{
  m_provides.push_back(sym.get());
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

} // namespace xboxdrv

/* EOF */
