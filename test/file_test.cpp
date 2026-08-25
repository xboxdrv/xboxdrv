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

#include <strut/file.hpp>

TEST(FileTest, readlines)
{
  EXPECT_EQ(strut::readlines("test/data/test.txt"),
            std::vector<std::string>({
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus",
                "laoreet quam eu porttitor sagittis. Etiam massa turpis, rutrum ac",
                "sollicitudin ac, fermentum ac libero. Pellentesque diam felis,",
                "tristique eget vulputate a, elementum in enim. Vivamus pulvinar lacus",
                "nec lorem imperdiet, vitae rutrum lorem vestibulum. Vivamus ac eros",
                "non mauris aliquam pharetra. Aenean molestie massa enim. Proin quam",
                "dolor, fermentum a sodales efficitur, ultrices sit amet massa.",
                "",
                "Aliquam erat volutpat. Donec accumsan, ipsum at tristique blandit,",
                "velit massa accumsan tellus, vitae rhoncus tortor purus viverra felis.",
                "Quisque tempus turpis vel auctor iaculis. Sed ut faucibus erat.",
                "Suspendisse tincidunt velit a tristique cursus. Etiam malesuada",
                "molestie ligula id imperdiet. Curabitur convallis erat quis ante",
                "accumsan efficitur. Integer dapibus tincidunt erat id tempor."
              }));

  EXPECT_EQ(strut::readlines("test/data/test_nonl.txt"),
            std::vector<std::string>({
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus",
                "laoreet quam eu porttitor sagittis. Etiam massa turpis, rutrum ac",
                "sollicitudin ac, fermentum ac libero. Pellentesque diam felis,",
                "tristique eget vulputate a, elementum in enim. Vivamus pulvinar lacus",
                "nec lorem imperdiet, vitae rutrum lorem vestibulum. Vivamus ac eros",
                "non mauris aliquam pharetra. Aenean molestie massa enim. Proin quam",
                "dolor, fermentum a sodales efficitur, ultrices sit amet massa.",
                "",
                "Aliquam erat volutpat. Donec accumsan, ipsum at tristique blandit,",
                "velit massa accumsan tellus, vitae rhoncus tortor purus viverra felis.",
                "Quisque tempus turpis vel auctor iaculis. Sed ut faucibus erat.",
                "Suspendisse tincidunt velit a tristique cursus. Etiam malesuada",
                "molestie ligula id imperdiet. Curabitur convallis erat quis ante",
                "accumsan efficitur. Integer dapibus tincidunt erat id tempor."
              }));
}

TEST(FileTest, readfile)
{
  EXPECT_EQ(strut::readfile("test/data/test.txt"),
            std::string(
              "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus\n"
              "laoreet quam eu porttitor sagittis. Etiam massa turpis, rutrum ac\n"
              "sollicitudin ac, fermentum ac libero. Pellentesque diam felis,\n"
              "tristique eget vulputate a, elementum in enim. Vivamus pulvinar lacus\n"
              "nec lorem imperdiet, vitae rutrum lorem vestibulum. Vivamus ac eros\n"
              "non mauris aliquam pharetra. Aenean molestie massa enim. Proin quam\n"
              "dolor, fermentum a sodales efficitur, ultrices sit amet massa.\n"
              "\n"
              "Aliquam erat volutpat. Donec accumsan, ipsum at tristique blandit,\n"
              "velit massa accumsan tellus, vitae rhoncus tortor purus viverra felis.\n"
              "Quisque tempus turpis vel auctor iaculis. Sed ut faucibus erat.\n"
              "Suspendisse tincidunt velit a tristique cursus. Etiam malesuada\n"
              "molestie ligula id imperdiet. Curabitur convallis erat quis ante\n"
              "accumsan efficitur. Integer dapibus tincidunt erat id tempor.\n"
              ));

  EXPECT_EQ(strut::readfile("test/data/test_nonl.txt"),
            std::string(
              "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus\n"
              "laoreet quam eu porttitor sagittis. Etiam massa turpis, rutrum ac\n"
              "sollicitudin ac, fermentum ac libero. Pellentesque diam felis,\n"
              "tristique eget vulputate a, elementum in enim. Vivamus pulvinar lacus\n"
              "nec lorem imperdiet, vitae rutrum lorem vestibulum. Vivamus ac eros\n"
              "non mauris aliquam pharetra. Aenean molestie massa enim. Proin quam\n"
              "dolor, fermentum a sodales efficitur, ultrices sit amet massa.\n"
              "\n"
              "Aliquam erat volutpat. Donec accumsan, ipsum at tristique blandit,\n"
              "velit massa accumsan tellus, vitae rhoncus tortor purus viverra felis.\n"
              "Quisque tempus turpis vel auctor iaculis. Sed ut faucibus erat.\n"
              "Suspendisse tincidunt velit a tristique cursus. Etiam malesuada\n"
              "molestie ligula id imperdiet. Curabitur convallis erat quis ante\n"
              "accumsan efficitur. Integer dapibus tincidunt erat id tempor."
              ));
}

/* EOF */
