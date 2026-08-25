// yaini - Yet another INI parser
// Copyright (C) 2010-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "schema_builder.hpp"

#include <stdexcept>

#include "schema.hpp"

namespace yaini {

SchemaBuilder::SchemaBuilder(const Schema& schema) :
  m_schema(schema),
  m_current_section()
{
}

void
SchemaBuilder::send_section(const std::string& section)
{
  m_current_section = section;
}

void
SchemaBuilder::send_pair(const std::string& name, const std::string& value)
{
  SchemaSection* section = m_schema.get_section(m_current_section);
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
      PairSchema* pair = section->get(name);
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

} // namespace yaini

/* EOF */
