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

#ifndef HEADER_STRUT_CASE_HPP
#define HEADER_STRUT_CASE_HPP

#include <cctype>
#include <string>
#include <string_view>

namespace strut {

inline std::string tolower(std::string_view text)
{
  std::string result(text);
  for(char& c : result) {
    c = static_cast<char>(::tolower(c));
  }
  return result;
}

inline std::string toupper(std::string_view text)
{
  std::string result(text);
  for(char& c : result) {
    c = static_cast<char>(::toupper(c));
  }
  return result;
}

} // namespace strut

#endif

/* EOF */
