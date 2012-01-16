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

#ifndef HEADER_XBOXDRV_SYMBOL_TABLE_HPP
#define HEADER_XBOXDRV_SYMBOL_TABLE_HPP

#include <map>
#include <string>

class SymbolTable
{
private:
  std::map<int, std::string> m_int2name;
  std::map<std::string, int> m_name2int;

public:
  SymbolTable();

  int put(const std::string& name);
  int put(const std::string& name, 
          const std::string& alias1);
  int put(const std::string& name,
          const std::string& alias1,
          const std::string& alias2);
  int put(const std::string& name,
          const std::string& alias1,
          const std::string& alias2,
          const std::string& alias3);

  /** Returns the id of the given name, on lookup failure the name is
      put into the table */
  int getput(const std::string& name);

  /** Returns the id of the given name, throws on lookup failure */
  int get(const std::string& name) const;

  int size() const { return m_int2name.size(); }
};

#endif

/* EOF */
