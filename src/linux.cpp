// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
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

#include "linux.hpp"

#include <cstdio>
#include <linux/input.h>
#include <unistd.h>
#include <vector>

namespace uinpp {

bool is_mouse_button(int ev_code)
{
  return  (ev_code >= BTN_MOUSE && ev_code <= BTN_TASK);
}

bool is_keyboard_button(int ev_code)
{
  return (ev_code < 256);
}

int find_jsdev_number()
{
  for(int i = 0; ; ++i)
  {
    char filename1[32];
    char filename2[32];

    sprintf(filename1, "/dev/input/js%d", i);
    sprintf(filename2, "/dev/js%d", i);

    if (access(filename1, F_OK) != 0 && access(filename2, F_OK) != 0)
      return i;
  }
}

int find_evdev_number()
{
  for(int i = 0; ; ++i)
  {
    char filename[32];

    sprintf(filename, "/dev/input/event%d", i);

    if (access(filename, F_OK) != 0)
      return i;
  }
}

} // namespace uinpp

/* EOF */
