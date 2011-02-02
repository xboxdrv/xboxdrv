/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "wakeup_pipe.hpp"

#include <errno.h>
#include <stdexcept>
#include <string.h>
#include <unistd.h>

#include "raise_exception.hpp"

WakeupPipe::WakeupPipe()
{
  int ret = pipe(m_pipefd);
  if (ret < 0)
  {
    raise_exception(std::runtime_error, "pipe() failed: " << strerror(errno));
  }
}

int
WakeupPipe::get_read_fd() const
{
  return m_pipefd[0];
}

int
WakeupPipe::get_write_fd() const
{
  return m_pipefd[1];
}

void
WakeupPipe::send_wakeup()
{
  char buf[1] = {0};
  ssize_t ret = write(get_write_fd(), buf, sizeof(buf));
  if (ret < 0)
  {
    raise_exception(std::runtime_error, "write() to pipe failed: " << strerror(errno));
  }
}

/* EOF */
