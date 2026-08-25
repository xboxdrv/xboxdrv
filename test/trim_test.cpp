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

#include <gtest/gtest.h>

#include <strut/trim.hpp>

TEST(TrimTest, trim)
{
  EXPECT_EQ(strut::trim("  Hello    "), "Hello");
  EXPECT_EQ(strut::trim("Hello    "), "Hello");
  EXPECT_EQ(strut::trim("    Hello"), "Hello");
  EXPECT_EQ(strut::trim("    Hello\n"), "Hello");
  EXPECT_EQ(strut::trim("    Hello\n\n\n"), "Hello");
}

/* EOF */
