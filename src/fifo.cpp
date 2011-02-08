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

#include "fifo.hpp"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>

#include "raise_exception.hpp"

Fifo::Fifo(const std::string& filename) :
  m_fd(),
  m_filename(filename)
{
  int ret = mkfifo(filename.c_str(), S_IRUSR | S_IWUSR);
  if (ret < 0)
  {
    raise_exception(std::runtime_error, "mkfifo() failed: " << strerror(errno));
  }
    
  m_fd = open(filename.c_str(), O_RDWR);
  if (m_fd < 0)
  {
    raise_exception(std::runtime_error, "open() of fifo failed: " << strerror(errno));
  }
}

Fifo::~Fifo()
{
  close(m_fd);

  int ret = unlink(m_filename.c_str());
  if (ret < 0)
  {
    // raise_exception(std::runtime_error, "unlink() of fifo failed: " << strerror(errno));
    // can't throw an exception
  }
}

int
Fifo::get_fd() const
{
  return m_fd;
}

std::string
Fifo::read()
{
  char data[12];
  ssize_t ret = ::read(m_fd, data, sizeof(data));
  if (ret < 0)
  {
    raise_exception(std::runtime_error, "read() from fifo failed: " << strerror(errno));
  }
  else if (ret == 0)
  {
    // nothing
    return std::string();
  }
  else
  {
    std::cout << "got " << ret << std::endl;
    std::cout << ">>";
    std::cout.write(data, ret);
    std::cout << "<<" << std::endl;
    return std::string(data, ret);
  }
}

#ifdef __TEST__
int main(int argc, char** argv)
{
  Fifo fifo("/tmp/test.fifo");

  while(true)
  {
    std::cout << "--------------------------------------------" << std::endl;
    fifo.read();
  }

  return 0;
}
#endif

/* EOF */
