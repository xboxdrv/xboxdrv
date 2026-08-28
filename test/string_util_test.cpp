/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include <utility>
#include <vector>

#include "util/string.hpp"
#include "unit_check.hpp"

using namespace xboxdrv;

static std::vector<std::pair<std::string, std::string>>
parse_pairs(std::string const& input)
{
  std::vector<std::pair<std::string, std::string>> out;
  process_name_value_string(input, [&](std::string const& k, std::string const& v) {
    out.emplace_back(k, v);
  });
  return out;
}

int main()
{
  {
    auto pairs = parse_pairs("foo=bar,baz=qux");
    CHECK_EQ(pairs.size(), static_cast<size_t>(2));
    CHECK_EQ(pairs[0].first, std::string("foo"));
    CHECK_EQ(pairs[0].second, std::string("bar"));
    CHECK_EQ(pairs[1].first, std::string("baz"));
    CHECK_EQ(pairs[1].second, std::string("qux"));
  }

  {
    auto pairs = parse_pairs("onlykey");
    CHECK_EQ(pairs.size(), static_cast<size_t>(1));
    CHECK_EQ(pairs[0].first, std::string("onlykey"));
    CHECK_EQ(pairs[0].second, std::string(""));
  }

  {
    auto pairs = parse_pairs("a=1,b=[x,y],c=3");
    CHECK_EQ(pairs.size(), static_cast<size_t>(3));
    CHECK_EQ(pairs[0].first, std::string("a"));
    CHECK_EQ(pairs[0].second, std::string("1"));
    CHECK_EQ(pairs[1].first, std::string("b"));
    CHECK_EQ(pairs[1].second, std::string("x,y"));
    CHECK_EQ(pairs[2].first, std::string("c"));
    CHECK_EQ(pairs[2].second, std::string("3"));
  }

  return test::exit_code();
}

/* EOF */
