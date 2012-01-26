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

#ifndef HEADER_XBOXDRV_ABS_SYMBOL_HPP
#define HEADER_XBOXDRV_ABS_SYMBOL_HPP

#include <string>

class AbsPort
{
protected:
  std::string m_name;
  int m_abs;
  
public:
  AbsPort(const std::string& name) :
    m_name(name),
    m_abs(-1)
  {}

  virtual ~AbsPort() 
  {}

  inline std::string get_name() const { return m_name; }
  inline int get_abs() const { return m_abs; }
};

class AbsPortIn : public AbsPort
{
public:
  AbsPortIn(const std::string& name) :
    AbsPort(name)
  {}
 
  void init(const ControllerMessageDescriptor& desc)
  {
    m_abs = desc.abs().get(get_name());
  }

  float get_float(const ControllerMessage& msg)
  {
    return msg.get_abs_float(get_abs());
  }

  int get(const ControllerMessage& msg)
  {
    return msg.get_abs(get_abs());
  }
};

class AbsPortOut : public AbsPort
{
public:
  AbsPortOut(const std::string& name) :
    AbsPort(name)
  {}

  void init(ControllerMessageDescriptor& desc)
  {
    m_abs = desc.abs().getput(get_name());
  }

  void set(ControllerMessage& msg, int value, int min, int max)
  {
    msg.set_abs(get_abs(), value, min, max);
  }

  void set_float(ControllerMessage& msg, float value)
  {
    msg.set_abs_float(get_abs(), value);
  }
};

#endif

/* EOF */
