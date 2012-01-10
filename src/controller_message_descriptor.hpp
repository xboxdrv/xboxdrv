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

#ifndef HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP
#define HEADER_XBOXDRV_CONTROLLER_MESSAGE_DESCRIPTOR_HPP

#include <map>
#include <string>

class ControllerMessageDescriptor
{
private:
  std::map<std::string, int> m_name2rel;
  std::map<std::string, int> m_name2key;
  std::map<std::string, int> m_name2abs;

public:
  ControllerMessageDescriptor();

  // registers a name and make it part of the ControllerMessage
  int register_key(const std::string& name);
  int register_abs(const std::string& name);
  int register_rel(const std::string& name);

  // returns the id associated with the given name, non-existing names
  // are a failure
  int get_key(const std::string& name) const;
  int get_abs(const std::string& name) const;
  int get_rel(const std::string& name) const;

  int get_key_count() const { return m_name2key.size(); }
  int get_abs_count() const { return m_name2abs.size(); }
  int get_rel_count() const { return m_name2rel.size(); }
  
private:
  ControllerMessageDescriptor(const ControllerMessageDescriptor&);
  ControllerMessageDescriptor& operator=(const ControllerMessageDescriptor&);
};

#endif

/* EOF */
