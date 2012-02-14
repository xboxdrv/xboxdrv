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

#ifndef HEADER_XBOXDRV_SYMBOLS_SYMBOL_HPP
#define HEADER_XBOXDRV_SYMBOLS_SYMBOL_HPP

#include <boost/shared_ptr.hpp>
#include <sstream>
#include <vector>

class Namespace;
class Symbol;

typedef boost::shared_ptr<Symbol> SymbolPtr;

class Symbol
{
private:
  Namespace& m_namespace;
  std::string m_name;
  std::vector<Symbol*> m_provides;

public:
  Symbol(Namespace& ns, const std::string& name);

  bool match(SymbolPtr sym) const;
  void add_provides(SymbolPtr sym);

  std::string get_namespace() const;
  std::string get_name() const;
  std::string str() const;
};

#endif

/* EOF */
