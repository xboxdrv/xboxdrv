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

#ifndef HEADER_YAINI_BUILDER_HPP
#define HEADER_YAINI_BUILDER_HPP

#include <string>

namespace yaini {

class Builder
{
public:
  virtual ~Builder() {}

  virtual void send_section(const std::string& section) =0;
  virtual void send_pair(const std::string& name, const std::string& value) =0;
};

} // namespace yaini

#endif

/* EOF */
