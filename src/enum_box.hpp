/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_ENUM_BOX_HPP
#define HEADER_XBOXDRV_ENUM_BOX_HPP

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace xboxdrv {

template<class Enum>
class EnumBox
{
protected:
  std::string m_name;
  std::map<Enum, std::string> m_enum2string;
  std::map<std::string, Enum> m_string2enum;

protected:
  EnumBox(const std::string& name) :
    m_name(name),
    m_enum2string(),
    m_string2enum()
  {
  }

  virtual ~EnumBox() {}

  void add(Enum i, const std::string& name)
  {
    m_enum2string[i] = name;
    m_string2enum[name] = i;
  }

public:
  typedef typename std::map<Enum, std::string>::iterator iterator;
  typedef typename std::map<Enum, std::string>::const_iterator const_iterator;

  const_iterator begin() const { return m_enum2string.begin(); }
  const_iterator end()   const { return m_enum2string.end();   }

  iterator begin() { return m_enum2string.begin(); }
  iterator end()   { return m_enum2string.end();   }

  Enum operator[](const std::string& str) const
  {
    typename std::map<std::string, Enum>::const_iterator i = m_string2enum.find(str);
    if (i == m_string2enum.end())
    {
      std::ostringstream out;
      out << "couldn't convert '" << str << "' to enum, not a member of " << m_name << std::endl;
      throw std::runtime_error(out.str());
    }
    else
    {
      return i->second;
    }
  }

  std::string operator[](Enum v) const {
    typename std::map<Enum, std::string>::const_iterator i = m_enum2string.find(v);
    if (i == m_enum2string.end())
    {
      std::ostringstream out;
      out << "couldn't convert '" << v << "' to string, not a member of " << m_name << std::endl;
      throw std::runtime_error(out.str());
    }
    else
    {
      return i->second;
    }
  }

  typename std::vector<Enum> get_values() const
  {
    std::vector<Enum> lst;
    for(const_iterator i = begin(); i != end(); ++i)
    {
      lst.push_back(i->first);
    }
    return lst;
  }

  std::vector<std::string> get_names() const
  {
    typename std::vector<std::string> lst;
    for(const_iterator i = begin(); i != end(); ++i)
    {
      lst.push_back(i->second);
    }
    return lst;
  }
};

} // namespace xboxdrv

#endif

/* EOF */
