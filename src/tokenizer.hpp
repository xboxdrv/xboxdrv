/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2025 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_TOKENIZER_HPP
#define HEADER_XBOXDRV_TOKENIZER_HPP

#include <string>
#include <vector>

std::vector<std::string> split_keep_empty(std::string const& text, std::string const& delims);
std::vector<std::string> split(const std::string& s, const std::string& delims);

template <typename Container>
std::string join(const Container& c, const std::string& delim)
{
  auto it = c.begin();
  auto end = c.end();

  if (it == end)
    return {};

  // Precompute size
  std::size_t total = 0;
  std::size_t count = 0;
  for (auto& s : c) {
    total += s.size();
    ++count;
  }
  if (count > 1)
    total += delim.size() * (count - 1);

  std::string out;
  out.reserve(total);

  // Append
  out += *it++;
  for (; it != end; ++it) {
    out += delim;
    out += *it;
  }

  return out;
}

#endif

/* EOF */
