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

#ifndef HEADER_XBOXDRV_SOCKET_HPP
#define HEADER_XBOXDRV_SOCKET_HPP

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "socket_ptr.hpp"

class Socket
{
public:
  /** Create a socked bound to a file in the filesytem */
  static SocketPtr create_unix(const std::string& filename);
  static SocketPtr connect_unix(const std::string& filename);

  /** Create a socket bound to an IP address, use '::' as hostname wildcard for
      both IPv6 and IPv4 addresses */
  static SocketPtr create_tcp(const std::string& host, const std::string& port);
  static SocketPtr connect_tcp(const std::string& host, const std::string& port);

private:
  int m_fd;

public:
  ~Socket();

  SocketPtr accept();
  int get_fd() const { return m_fd; }

  std::string recv();
  void send(const std::string& data);

private:
  Socket(int fd);
  Socket(int domain, int type);

  void connect(const struct sockaddr *addr, socklen_t addrlen);
  void bind(const struct sockaddr *addr, socklen_t addrlen);
  void listen();
  
private:
  Socket(const Socket&);
  Socket& operator=(const Socket&);
};

#endif

/* EOF */
