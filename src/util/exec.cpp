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

#include "util/exec.hpp"

#include <assert.h>
#include <string.h>

#include <logmich/log.hpp>

namespace xboxdrv {

pid_t spawn_exe(const std::string& arg0)
{
  std::vector<std::string> args;
  args.push_back(arg0);
  return spawn_exe(args);
}

pid_t spawn_exe(const std::vector<std::string>& args)
{
  assert(!args.empty());

  pid_t pid = fork();
  if (pid == 0)
  {
    char** argv = static_cast<char**>(malloc(sizeof(char*) * (args.size() + 1)));
    for(size_t i = 0; i < args.size(); ++i)
    {
      argv[i] = strdup(args[i].c_str());
    }
    argv[args.size()] = NULL;

    if (execvp(args[0].c_str(), argv) == -1)
    {
      log_error("{}: exec failed: {}", args[0], strerror(errno));
      _exit(EXIT_FAILURE);
    }
  }

  return pid;
}

} // namespace xboxdrv

/* EOF */
