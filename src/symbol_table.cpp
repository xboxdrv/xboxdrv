/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "symbol_table.hpp"

#include <stdexcept>
#include <iostream>
#include <ctype.h>

#include "raise_exception.hpp"

namespace {

std::string tolower(const std::string& str)
{
  std::string ret = str;
  for(std::string::size_type i = 0; i < ret.size(); ++i)
  {
    ret[i] = static_cast<char>(::tolower(ret[i]));
  }
  return ret;
}

} // namespace

SymbolTable::SymbolTable() :
  m_int2name(),
  m_name2int()
{
}

int 
SymbolTable::put(const std::string& name_)
{
  //std::cout << "SymbolTable::put(" << name << ")" << std::endl;
  std::string name = tolower(name_);

  int symbol = m_int2name.size();
  m_int2name.push_back(name);
  m_name2int[name] = symbol;
  return symbol;
}

int 
SymbolTable::put(const std::string& name_, 
                 const std::string& alias1)
{
  std::string name = tolower(name_);

  int symbol = m_int2name.size();
  m_int2name.push_back(name);
  m_name2int[name] = symbol;
  m_name2int[tolower(alias1)] = symbol;
  return symbol;
}

int 
SymbolTable::put(const std::string& name_,
                 const std::string& alias1,
                 const std::string& alias2)
{
  std::string name = tolower(name_);

  int symbol = m_int2name.size();
  m_int2name.push_back(name);
  m_name2int[name] = symbol;
  m_name2int[tolower(alias1)] = symbol;
  m_name2int[tolower(alias2)] = symbol;
  return symbol;
}

int 
SymbolTable::put(const std::string& name_,
                 const std::string& alias1,
                 const std::string& alias2,
                 const std::string& alias3)
{
  std::string name = tolower(name_);

  int symbol = m_int2name.size();
  m_int2name.push_back(name);
  m_name2int[name] = symbol;
  m_name2int[tolower(alias1)] = symbol;
  m_name2int[tolower(alias2)] = symbol;
  m_name2int[tolower(alias3)] = symbol;
  return symbol;
}

int 
SymbolTable::put(const std::string& name_,
                 const std::string& alias1,
                 const std::string& alias2,
                 const std::string& alias3,
                 const std::string& alias4)
{
  std::string name = tolower(name_);

  int symbol = m_int2name.size();
  m_int2name.push_back(name);
  m_name2int[name] = symbol;
  m_name2int[tolower(alias1)] = symbol;
  m_name2int[tolower(alias2)] = symbol;
  m_name2int[tolower(alias3)] = symbol;
  m_name2int[tolower(alias4)] = symbol;
  return symbol;
}

int 
SymbolTable::getput(const std::string& name_)
{
  std::string name = tolower(name_);

  std::map<std::string, int>::const_iterator it = m_name2int.find(name);
  if (it == m_name2int.end())
  {
    return put(name);
  }
  else
  {
    return it->second;
  }
}

int 
SymbolTable::get(const std::string& name_) const
{
  std::string name = tolower(name_);

  std::map<std::string, int>::const_iterator it = m_name2int.find(name);
  if (it == m_name2int.end())
  {
    raise_exception(std::runtime_error, "lookup failure for: '" << name << "'");
  }
  else
  {
    return it->second;
  }
}

std::string
SymbolTable::get(int v) const
{
  return m_int2name.at(v);
}

/* EOF */
