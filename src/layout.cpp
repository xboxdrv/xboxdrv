// strut - Collection of string utilities
// Copyright (C) 2011-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "layout.hpp"

#include <ostream>

#include "tokenize.hpp"

namespace strut {

Layout::Layout(std::ostream& out, int width) :
  m_out(out),
  m_width(width)
{
}

void
Layout::println(std::string const& str)
{
  m_out << str << std::endl;
}

void
Layout::newline()
{
  m_out << std::endl;
}

void
Layout::para(std::string const& str) const
{
  para("", str);
}

void
Layout::para(std::string const& prefix, std::string const& str) const
{
  auto tokens = strut::tokenize(str, ' ');

  int len = static_cast<int>(prefix.size());
  m_out << prefix;
  for(auto i = tokens.begin(); i != tokens.end(); ++i)
  {
    if (len + static_cast<int>(i->size()) + 1 >= m_width)
    {
      m_out << std::endl;
      m_out << prefix;
      len = static_cast<int>(prefix.size());
    }

    m_out << *i << " ";
    len += static_cast<int>(i->size()) + 1;
  }
  m_out << std::endl;
}

} // namespace strut

/* EOF */
