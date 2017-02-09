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

#ifndef HEADER_XBOXDRV_SYMBOL_TABLE_HPP
#define HEADER_XBOXDRV_SYMBOL_TABLE_HPP

#include <ctype.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

#include "raise_exception.hpp"

template<typename T>
class SymbolTable
{
private:
  typename std::vector<T> m_int2name;
  typename std::map<T, int> m_name2int;

public:
  SymbolTable() :
    m_int2name(),
    m_name2int()
  {
  }

  int put(const T& name)
  {
    int symbol = static_cast<int>(m_int2name.size());
    m_int2name.push_back(name);
    m_name2int[name] = symbol;
    return symbol;
  }

  /** Returns the id of the given name, on lookup failure the name is
      put into the table */
  int getput(const T& name)
  {
    typename std::map<T, int>::const_iterator it = m_name2int.find(name);
    if (it == m_name2int.end())
    {
      return put(name);
    }
    else
    {
      return it->second;
    }
  }

  /** Returns the id of the given name, throws on lookup failure */
  int get(const T& name) const
  {
    typename std::map<T, int>::const_iterator it = m_name2int.find(name);
    if (it == m_name2int.end())
    {
      raise_exception(std::runtime_error, "lookup failure for: '" << name << "'");
    }
    else
    {
      return it->second;
    }
  }

  T get(int v) const
  {
    return m_int2name.at(v);
  }

  bool has(const T& name) const
  {
    return m_name2int.find(name) != m_name2int.end();
  }

  int size() const { return static_cast<int>(m_int2name.size()); }
};

#endif

/* EOF */
