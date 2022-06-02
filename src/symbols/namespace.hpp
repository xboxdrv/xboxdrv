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

#ifndef HEADER_XBOXDRV_SYMBOLS_NAMESPACE_HPP
#define HEADER_XBOXDRV_SYMBOLS_NAMESPACE_HPP

#include <memory>
#include <map>
#include <string>
#include <stdexcept>

#include "raise_exception.hpp"
#include "symbol.hpp"

namespace xboxdrv {

class Namespace
{
private:
  std::string m_name;
  typedef std::map<std::string, SymbolPtr> Symbols;
  Symbols m_symbols;

public:
  Namespace(const std::string& name);

  std::string get_name() const { return m_name; }

  SymbolPtr lookup(const std::string& name);
  SymbolPtr add_symbol(const std::string& name);
  void add_alias(const std::string& name, SymbolPtr sym);

private:
  Namespace(const Namespace&);
  Namespace& operator=(const Namespace&);
};

typedef std::shared_ptr<Namespace> NamespacePtr;

} // namespace xboxdrv

#endif

/* EOF */
