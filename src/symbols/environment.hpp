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

#ifndef HEADER_XBOXDRV_SYMBOLS_ENVIRONMENT_HPP
#define HEADER_XBOXDRV_SYMBOLS_ENVIRONMENT_HPP

#include <string>
#include <map>

#include "namespace.hpp"

class Environment
{
private:
  typedef std::map<std::string, NamespacePtr> Namespaces;
  Namespaces m_namespaces;

public:
  Environment() :
    m_namespaces()
  {
  }

  NamespacePtr lookup_namespace(const std::string& ns)
  {
    Namespaces::iterator it = m_namespaces.find(ns);
    if (it == m_namespaces.end())
    {
      return NamespacePtr();
    }    
    else
    {
      return it->second;
    }
  }

  SymbolPtr lookup_symbol(const std::string& ns, const std::string& symbol)
  {
    Namespaces::iterator it = m_namespaces.find(ns);
    if (it == m_namespaces.end())
    {
      return SymbolPtr();
    }
    else
    {
      return it->second->lookup(symbol);
    }
  }

  NamespacePtr add_namespace(const std::string& name)
  {
    Namespaces::iterator it = m_namespaces.find(name);
    if (it != m_namespaces.end())
    {
      raise_exception(std::runtime_error, "name conflict for symbol: " << name);
    }
    else
    {
      NamespacePtr ns(new Namespace(name));
      m_namespaces[name] = ns;
      return ns;
    }
  }

  void add_namespace_alias(const std::string& name, NamespacePtr ns)
  {
    Namespaces::iterator it = m_namespaces.find(name);
    if (it != m_namespaces.end())
    {
      raise_exception(std::runtime_error, "name conflict for symbol: " << name);
    }
    else
    {
      m_namespaces[name] = ns;
    }
  }

private:
  Environment(const Environment&);
  Environment& operator=(const Environment&);
};

typedef boost::shared_ptr<Environment> EnvironmentPtr;

#endif

/* EOF */
