// strut - Collection of string utilities
// Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_STRUT_JOIN_HPP
#define HEADER_STRUT_JOIN_HPP

#include <string_view>

namespace strut {

template<typename C>
std::string join(C const& c, std::string_view sep)
{
  std::string result;
  auto it = std::begin(c);
  if (it == std::end(c))
  {
    return {};
  }
  else
  {
    result += *it;
    ++it;
    for(; it != std::end(c); ++it)
    {
      result += sep;
      result += *it;
    }
    return result;
  }
}

} // namespace strut

#endif

/* EOF */
