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

#include "tokenizer.hpp"

std::vector<std::string> split_keep_empty(std::string const& text, std::string const& delims)
{
  std::vector<std::string> result;
  std::string token;

  for (size_t i = 0; i < text.size(); ++i) {
    if (delims.find(text[i]) != std::string::npos) {
      result.push_back(token);
      token.clear();
    } else {
      token += text[i];
    }
  }

  result.push_back(token);

  return result;
}

std::vector<std::string> split(const std::string& s, const std::string& delims = " ")
{
  std::vector<std::string> result;
  size_t start = 0;

  while (start < s.size()) {
    size_t pos = s.find_first_of(delims, start);
    if (pos != start) {
      result.push_back(s.substr(start, pos - start));
    }
    if (pos == std::string::npos)
      break;
    start = pos + 1;
  }

  return result;
}

/* EOF */
