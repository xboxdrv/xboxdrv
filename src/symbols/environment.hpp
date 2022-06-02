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

#ifndef HEADER_XBOXDRV_SYMBOLS_ENVIRONMENT_HPP
#define HEADER_XBOXDRV_SYMBOLS_ENVIRONMENT_HPP

#include <string>
#include <map>

#include "namespace.hpp"

namespace xboxdrv {

class Environment
{
private:
  typedef std::map<std::string, NamespacePtr> Namespaces;
  Namespaces m_namespaces;

public:
  Environment();

  NamespacePtr lookup_namespace(const std::string& ns);
  SymbolPtr lookup_symbol(const std::string& ns, const std::string& symbol);
  NamespacePtr add_namespace(const std::string& name);
  void add_namespace_alias(const std::string& name, NamespacePtr ns);

private:
  Environment(const Environment&);
  Environment& operator=(const Environment&);
};

typedef std::shared_ptr<Environment> EnvironmentPtr;

} // namespace xboxdrv

#endif

/* EOF */
