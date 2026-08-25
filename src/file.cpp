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

#include "file.hpp"

#include <sstream>
#include <fstream>
#include <string.h>
#include <errno.h>

namespace strut {

std::string readfile(std::filesystem::path const& filename)
{
  std::ifstream fin(filename);
  if (!fin) {
    std::ostringstream oss;
    oss << filename << ": failed to open: " << strerror(errno);
    throw std::runtime_error(oss.str());
  }

  size_t bytes_read = 0;
  constexpr size_t blocksize = 1024 * 64;
  std::string result;
  while(!fin.eof()) {
    result.resize(result.size() + blocksize);
    fin.read(result.data() + bytes_read, blocksize);
    bytes_read += fin.gcount();
    if (fin.gcount() < blocksize) {
      result.resize(result.size() - blocksize + fin.gcount());
    }
  }
  return result;
}

std::vector<std::string> readlines(std::filesystem::path const& filename)
{
  std::ifstream fin(filename);
  if (!fin) {
    std::ostringstream oss;
    oss << filename << ": failed to open: " << strerror(errno);
    throw std::runtime_error(oss.str());
  }

  std::string line;
  std::vector<std::string> result;
  while(std::getline(fin, line)) {
    result.emplace_back(std::move(line));
  }
  return result;
}

} // namespace strut

/* EOF */
