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

#include <sstream>

#include <gtest/gtest.h>

#include <strut/layout.hpp>

TEST(LayoutTest, layout)
{
  int const text_width = 80;
  std::ostringstream os;
  strut::Layout layout(os, text_width);

  layout.println("Title");
  layout.newline();

  layout.para("Lorem ipsum dolor sit amet, consequatectetur adipiscing elit, sed do "
              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad "
              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
              "aliquip ex ea commodo consequat. Duis aute irure dolor in "
              "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
              "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
              "culpa qui officia deserunt mollit anim id est laborum.");
  layout.newline();

  layout.para("> ",
              "Lorem ipsum dolor sit amet, consequatectetur adipiscing elit, sed do "
              "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad "
              "minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
              "aliquip ex ea commodo consequat. Duis aute irure dolor in "
              "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
              "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
              "culpa qui officia deserunt mollit anim id est laborum.");
  layout.newline();

  std::string const result = os.str();
  EXPECT_EQ(result,
            "Title\n"
            "\n"
            "Lorem ipsum dolor sit amet, consequatectetur adipiscing elit, sed do eiusmod \n"
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, \n"
            "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo \n"
            "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse \n"
            "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non \n"
            "proident, sunt in culpa qui officia deserunt mollit anim id est laborum. \n"
            "\n"
            "> Lorem ipsum dolor sit amet, consequatectetur adipiscing elit, sed do eiusmod \n"
            "> tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, \n"
            "> quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo \n"
            "> consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse \n"
            "> cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat \n"
            "> non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. \n"
            "\n");
}

/* EOF */
