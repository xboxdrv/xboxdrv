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

#ifndef HEADER_XBOXDRV_KEY_SYMBOL_HPP
#define HEADER_XBOXDRV_KEY_SYMBOL_HPP

#include <string>

#include "controller_message.hpp"
#include "controller_message_descriptor.hpp"

class KeyPort
{
protected:
  std::string m_name;
  int m_key;
  
public:
  KeyPort(const std::string& name) :
    m_name(name),
    m_key(-1)
  {}
  virtual ~KeyPort() {}

  inline std::string get_name() const { return m_name; }
  inline int get_key() const { return m_key; }
  inline std::string str() const { return m_name; }
};

class KeyPortIn : public KeyPort
{
public:
  KeyPortIn(const std::string& name) :
    KeyPort(name)
  {}

  void init(const ControllerMessageDescriptor& desc)
  {
    m_key = desc.key().get(m_name);
  }

  int get(const ControllerMessage& msg)
  {
    return msg.get_key(m_key);
  }
};

class KeyPortOut : public KeyPort
{
public:
  KeyPortOut(const std::string& name) :
    KeyPort(name)
  {}

  void init(ControllerMessageDescriptor& desc)
  {
    m_key = desc.key().put(m_name);
  }

  void set(ControllerMessage& msg, int value)
  {
    msg.set_key(get_key(), value);
  }
};

#endif

/* EOF */
