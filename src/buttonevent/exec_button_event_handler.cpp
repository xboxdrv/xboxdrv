/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "exec_button_event_handler.hpp"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <logmich/log.hpp>
#include "util/string.hpp"

ExecButtonEventHandler*
ExecButtonEventHandler::from_string(const std::string& str)
{
  return new ExecButtonEventHandler(string_split(str, ":"));
}

ExecButtonEventHandler::ExecButtonEventHandler(const std::vector<std::string>& args) :
  m_args(args)
{
}

void
ExecButtonEventHandler::send(bool value)
{
  if (!value)
  {
    return;
  }

  pid_t tmp_pid = fork();
  if (tmp_pid == 0)
  {
    // Double fork to reap the child and disown the execed process
    pid_t pid = fork();

    if (pid == 0)
    {
      char** argv = static_cast<char**>(malloc(sizeof(char*) * (m_args.size() + 1)));
      for(size_t i = 0; i < m_args.size(); ++i)
      {
        argv[i] = strdup(m_args[i].c_str());
      }
      argv[m_args.size()] = NULL;

      if (execvp(m_args[0].c_str(), argv) == -1)
      {
        log_error("exec failed: {}", strerror(errno));
        _exit(EXIT_FAILURE);
      }
    }

    _exit(EXIT_SUCCESS);
  }

  waitpid(tmp_pid, NULL, 0);
}

std::string
ExecButtonEventHandler::str() const
{
  return "exec";
}

/* EOF */
