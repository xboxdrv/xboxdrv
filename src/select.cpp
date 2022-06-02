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

#include "select.hpp"

#include <errno.h>
#include <string.h>
#include <stdexcept>

#include "raise_exception.hpp"

namespace xboxdrv {

#pragma GCC diagnostic ignored "-Wold-style-cast"

Select::Select() :
  m_nfds(0),
  m_readfds()
{
  FD_ZERO(&m_readfds);
}

void
Select::clear()
{
  m_nfds = 0;
  FD_ZERO(&m_readfds);
}

void
Select::add_fd(int fd)
{
  FD_SET(fd, &m_readfds);
  m_nfds = std::max(m_nfds, fd);
}

bool
Select::is_ready(int fd) const
{
  return FD_ISSET(fd, &m_readfds);
}

int
Select::wait()
{
  int ret = select(m_nfds+1,
                   &m_readfds,
                   NULL /* writefds */,
                   NULL /* exceptfds */,
                   NULL /* utimeout */);

  if (ret < 0)
  {
    raise_exception(std::runtime_error, "select() call failed: " << strerror(errno));
  }
  else
  {
    // return number of fds that have data on them
    return ret;
  }
}

} // namespace xboxdrv

/* EOF */
