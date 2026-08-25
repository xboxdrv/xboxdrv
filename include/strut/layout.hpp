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

#ifndef HEADER_STRUT_LAYOUT_HPP
#define HEADER_STRUT_LAYOUT_HPP

#include <iosfwd>
#include <string>

namespace strut {

class Layout
{
public:
  Layout(std::ostream& m_out, int width);

  void para(std::string const& str) const;
  void para(std::string const& prefix, std::string const& str) const;
  void println(std::string const& str);
  void newline();

private:
  std::ostream& m_out;
  int const m_width;

private:
  Layout(Layout const&);
  Layout& operator=(Layout const&);
};

} // namespace strut

#endif

/* EOF */
