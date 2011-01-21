/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_INI_SCHEMA_BUILDER_HPP
#define HEADER_XBOXDRV_INI_SCHEMA_BUILDER_HPP

#include "ini_builder.hpp"

class INISchema;

class INISchemaBuilder : public INIBuilder
{
private:
  const INISchema& m_schema;
  std::string m_current_section;

public:
  INISchemaBuilder(const INISchema& schema);

  void send_section(const std::string& section);
  void send_pair(const std::string& name, const std::string& value);

private:
  INISchemaBuilder(const INISchemaBuilder&);
  INISchemaBuilder& operator=(const INISchemaBuilder&);
};

#endif

/* EOF */
