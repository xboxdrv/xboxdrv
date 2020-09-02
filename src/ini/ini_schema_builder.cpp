/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "ini/ini_schema_builder.hpp"

#include <stdexcept>

#include "ini/ini_schema.hpp"

INISchemaBuilder::INISchemaBuilder(const INISchema& schema) :
  m_schema(schema),
  m_current_section()
{
}

void
INISchemaBuilder::send_section(const std::string& section)
{
  m_current_section = section;
}

void
INISchemaBuilder::send_pair(const std::string& name, const std::string& value)
{
  INISchemaSection* section = m_schema.get_section(m_current_section);
  if (!section)
  {
    throw std::runtime_error("unknown section: '" + m_current_section + "'");
  }
  else
  {
    if (section->m_callback)
    {
      section->m_callback(name, value);
    }
    else
    {
      INIPairSchema* pair = section->get(name);
      if (!pair)
      {
        throw std::runtime_error("unknown name: '" + name + "'");
      }
      else
      {
        pair->call(value);
      }
    }
  }
}

/* EOF */
