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

#include <strut/to_string.hpp>

TEST(ToStringTest, to_string)
{
  EXPECT_EQ(strut::to_string(123), "123");
  EXPECT_EQ(strut::to_string(1.25f), "1.25");
  EXPECT_EQ(strut::to_string("Hello World"), "Hello World");
}

TEST(ToStringTest, type_to_string)
{
  EXPECT_EQ(strut::to_string<short>(), "short");
  EXPECT_EQ(strut::to_string<int>(), "int");
  EXPECT_EQ(strut::to_string<long>(), "long");
  EXPECT_EQ(strut::to_string<float>(), "float");
  EXPECT_EQ(strut::to_string<double>(), "double");
}

/* EOF */
