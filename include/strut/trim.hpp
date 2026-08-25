// strut - Collection of string utilities
// Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_STRUT_TRIM_HPP
#define HEADER_STRUT_TRIM_HPP

#include <cctype>

namespace strut {

inline
std::string_view trim_view(std::string_view text)
{
  std::string::size_type beg = 0;
  while(std::isspace(text[beg]) && beg < text.size()) {
    beg += 1;
  }

  std::string::size_type back = text.size() - 1;
  while(std::isspace(text[back]) && back >= 1) {
    back -= 1;
  }

  return std::string_view(text.begin() + beg, text.begin() + back + 1);
}

inline
std::string trim(std::string_view text)
{
  return std::string(trim_view(text));
}

} // namespace strut

#endif

/* EOF */
