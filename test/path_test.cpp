/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "path.hpp"
#include "unit_check.hpp"

using namespace xboxdrv;

int main()
{
  CHECK_EQ(path::dirname("file.txt"), std::string("./"));
  CHECK_EQ(path::dirname("/usr/bin/xboxdrv"), std::string("/usr/bin/"));
  CHECK_EQ(path::dirname("/usr/bin/"), std::string("/usr/bin/"));
  CHECK_EQ(path::dirname("rel/path/file"), std::string("rel/path/"));

  CHECK_EQ(path::join("a", "b"), std::string("a/b"));
  CHECK_EQ(path::join("a/", "b"), std::string("a/b"));
  CHECK_EQ(path::join("", "b"), std::string("b"));

  return test::exit_code();
}

/* EOF */
