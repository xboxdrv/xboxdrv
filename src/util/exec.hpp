/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2020 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_UTIL_EXEC_HPP
#define HEADER_XBOXDRV_UTIL_EXEC_HPP

#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

pid_t spawn_exe(const std::vector<std::string>& args);
pid_t spawn_exe(const std::string& arg0);

#endif

/* EOF */
