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

#include "controller_message_descriptor.hpp"

#include <stdexcept>

#include "raise_exception.hpp"

ControllerMessageDescriptor::ControllerMessageDescriptor() :
  m_name2rel(),
  m_name2key(),
  m_name2abs()
{
}

int
ControllerMessageDescriptor::register_key(const std::string& name)
{
  std::map<std::string, int>::const_iterator it = m_name2key.find(name);
  if (it == m_name2key.end())
  {
    int id = m_name2key.size();
    m_name2key[name] = id;
    return id;
  }
  else
  {
    return it->second;
  }
}

int
ControllerMessageDescriptor::register_abs(const std::string& name)
{
  std::map<std::string, int>::const_iterator it = m_name2abs.find(name);
  if (it == m_name2abs.end())
  {
    int id = m_name2abs.size();
    m_name2abs[name] = id;
    return id;
  }
  else
  {
    return it->second;
  }
}

int
ControllerMessageDescriptor::register_rel(const std::string& name)
{
  std::map<std::string, int>::const_iterator it = m_name2rel.find(name);
  if (it == m_name2rel.end())
  {
    int id = m_name2rel.size();
    m_name2rel[name] = id;
    return id;
  }
  else
  {
    return it->second;
  }
}

int
ControllerMessageDescriptor::get_key(const std::string& name) const
{
  std::map<std::string, int>::const_iterator it = m_name2key.find(name);
  if (it == m_name2key.end())
  {
    raise_exception(std::runtime_error, "not a valid key name: " << name);
  }
  else
  {
    return it->second;
  }
}

int
ControllerMessageDescriptor::get_abs(const std::string& name) const
{
  std::map<std::string, int>::const_iterator it = m_name2abs.find(name);
  if (it == m_name2abs.end())
  {
    raise_exception(std::runtime_error, "not a valid abs name: " << name);
  }
  else
  {
    return it->second;
  }
}

int
ControllerMessageDescriptor::get_rel(const std::string& name) const
{
  std::map<std::string, int>::const_iterator it = m_name2rel.find(name);
  if (it == m_name2rel.end())
  {
    raise_exception(std::runtime_error, "not a valid rel name: " << name);
  }
  else
  {
    return it->second;
  }
}

/* EOF */
