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

#ifndef HEADER_XBOXDRV_SYMBOLS_NAME_HPP
#define HEADER_XBOXDRV_SYMBOLS_NAME_HPP

#include <string>
#include <boost/serialization/strong_typedef.hpp>
#include <iostream>

#include "symbols/environment.hpp"
#include "symbols/symbol.hpp"

template<void (*init_environment)(EnvironmentPtr)>
class Name
{
private:
  static EnvironmentPtr s_env;

public:
  Name() :
    m_symbol()
  {
  }

  Name(const std::string& name) :
    m_symbol()
  {
    if (!s_env)
    {
      s_env.reset(new Environment);
      init_environment(s_env);
    }

    std::string::size_type p = name.find('.');
    if (p == std::string::npos)
    {
      // unqualified name
      raise_exception(std::runtime_error, "user variables not implemented yet: " << name);
    }
    else
    {
      // fully qualified name
      m_symbol = s_env->lookup_symbol(name.substr(0, p), name.substr(p + 1));
      if (!m_symbol)
      {
        raise_exception(std::runtime_error, "unknown name: " << name);
      }
    }
  }

  operator void*() const
  {
    return m_symbol.get();
  }

  std::string str() const
  {
    if (m_symbol)
    {
      return m_symbol->str();
    }
    else
    {
      return "(null)";
    }
  }

  bool match(const Name& sym) const
  {
    if (m_symbol)
    {
      return m_symbol->match(sym.m_symbol);
    }
    else
    {
      return false;
    }
  }

private:
  SymbolPtr m_symbol;
};

template<void (*init_environment)(EnvironmentPtr)>
std::ostream& operator<<(std::ostream& os, const Name<init_environment>& data)
{
  return os << data.str();
}

template<void (*init_environment)(EnvironmentPtr)> EnvironmentPtr Name<init_environment>::s_env;

void init_environment_abs(EnvironmentPtr env);
void init_environment_key(EnvironmentPtr env);
void init_environment_rel(EnvironmentPtr env);

typedef Name<init_environment_abs> AbsName;
typedef Name<init_environment_key> KeyName;
typedef Name<init_environment_rel> RelName;

#endif

/* EOF */
