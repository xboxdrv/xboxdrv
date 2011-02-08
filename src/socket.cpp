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

#include "socket.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdexcept>
#include <string.h>
#include <netdb.h>

#include "raise_exception.hpp"

SocketPtr
Socket::create_unix(const std::string& filename)
{
  if (filename.size() >= sizeof(sockaddr_un::sun_path))
  {
    raise_exception(std::runtime_error, "socket filename is beyond maximum length");
  }
  
  // FIXME: most other apps store their sockets into a directory,
  // instead of directly, why?
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, filename.c_str());
  SocketPtr sock(new Socket(AF_UNIX, SOCK_STREAM));

  // FIXME: feels dirty to just overwrite the already exist socket
  // file, but seems to be the only way to avoid potential "address
  // already in use" issues, might still be a good idea to try to
  // clean up the socket properly in the destructor
  unlink(filename.c_str());
  
  int prev_mask = umask(0177); // only allow u+rw bits
  sock->bind(reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_un));
  umask(prev_mask);

  sock->listen();
  return sock;
}

SocketPtr
Socket::connect_unix(const std::string& filename)
{
  if (filename.size() >= sizeof(sockaddr_un::sun_path))
  {
    raise_exception(std::runtime_error, "socket filename is beyond maximum length");
  }
  
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, filename.c_str());

  SocketPtr sock(new Socket(AF_UNIX, SOCK_STREAM));
  sock->connect(reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_un));
  return sock;
}

SocketPtr
Socket::create_tcp(const std::string& host,
                   const std::string& port)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family    = AF_UNSPEC;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = 0;
  hints.ai_flags     = AI_PASSIVE;
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;

  addrinfo* result;
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0)
  {
    raise_exception(std::runtime_error, "getaddrinfo() failed: " << gai_strerror(errno));
  }
  else
  {
    int sfd = -1;
    addrinfo* rp;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      // FIXME: must make sure that this is close()'ed and not ignored due to exceptions
      // FIXME: not checking return value
      sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      
      if (::bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
        break; // success

      close(sfd);
    }
    freeaddrinfo(result);
    
    if (rp == NULL)
    {
      raise_exception(std::runtime_error, "failed to bind to: " << host << ":" << port);
    }
    else
    {
      SocketPtr sock(new Socket(sfd));

      // get rid of TIME_WAIT
      int on = 1;
      if (setsockopt(sock->get_fd(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
      {
        raise_exception(std::runtime_error, "setsockopt(SO_REUSEADDR) failed: " << strerror(errno));
      }

      sock->listen();
      return sock;
    }
  }
}

SocketPtr
Socket::connect_tcp(const std::string& host,
                    const std::string& port)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family    = AF_UNSPEC;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_protocol  = 0;
  hints.ai_flags     = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;

  addrinfo* result;
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0)
  {
    raise_exception(std::runtime_error, "getaddrinfo() failed: " << gai_strerror(errno));
  }
  else
  {
    int sfd = -1;
    addrinfo* rp;
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      // FIXME: not checking return value
      sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      
      if (::connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
        break; // success

      close(sfd);
    }
    freeaddrinfo(result);
    
    if (rp == NULL)
    {
      raise_exception(std::runtime_error, "failed to connect to: " << host << ":" << port);
    }
    else
    {
      return SocketPtr(new Socket(sfd));
    }
  }
}

Socket::Socket(int fd) :
  m_fd(fd)
{
}

Socket::Socket(int domain, int type) :
  m_fd(-1)
{
  m_fd = socket(domain, type, 0);
  if (m_fd < 0)
  {
    raise_exception(std::runtime_error, "couldn't create socket: " << strerror(errno));
  }
}

Socket::~Socket()
{
  if (shutdown(m_fd, SHUT_RDWR) != 0)
  {
    std::cout << "Socket::~Socket(): shutdown() failed: " << strerror(errno) << std::endl;;
  }

  if (close(m_fd) != 0)
  {
    std::cout << "Socket::~Socket(): close() failed: " << strerror(errno) << std::endl;;
  }
}

void
Socket::connect(const struct sockaddr *addr, socklen_t addr_len)
{
  int ret = ::connect(m_fd, reinterpret_cast<sockaddr*>(&addr), addr_len);
  if (ret != 0)
  {
    raise_exception(std::runtime_error, "connect() failed: " << strerror(errno));
  }
}

void
Socket::listen()
{
  int ret = ::listen(m_fd, 32);
  if (ret != 0)
  {
    raise_exception(std::runtime_error, "listen() failed: " << strerror(errno));    
  }
}

void
Socket::bind(const struct sockaddr *addr, socklen_t addr_len)
{
  if (::bind(m_fd, addr, addr_len) != 0)
  {
    raise_exception(std::runtime_error, "bind() failed: " << strerror(errno));
  }
}

SocketPtr
Socket::accept()
{
  //sockaddr  addr;
  // socklen_t addr_len;
  int sock = ::accept(m_fd, NULL, NULL);
  if (sock < 0)
  {
    raise_exception(std::runtime_error, "accept() failed: " << strerror(errno));
  }
  else
  {
    return SocketPtr(new Socket(sock));
  }
}

void
Socket::send(const std::string& data)
{
  ssize_t len = ::send(m_fd, data.c_str(), data.size(), MSG_NOSIGNAL);
  // can fail with "Broken Pipe" - EPIPE
  if (len < 0)
  {
    raise_exception(std::runtime_error, "send() failed: " << strerror(errno));
  }
  else if (len != static_cast<ssize_t>(data.size()))
  {
    raise_exception(std::runtime_error, "short send(): " << data.size() << " != " << len << ": " << strerror(errno));
  }
}

std::string
Socket::recv()
{
  char buf[1024];
  
  int len = ::recv(m_fd, buf, sizeof(buf), 0 /*flags*/);

  if (len < 0)
  {
    raise_exception(std::runtime_error, "recv() failed: " << strerror(errno));
  }
  else if (len == 0)
  {
    raise_exception(std::runtime_error, "recv() failed: peer shutdown");
  }
  else
  {
    return std::string(buf, len);
  }
}

#ifdef __TEST__

#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
  try 
  {
    if (argc == 2)
    {
      std::cout << "creating unix domain socket at: " << argv[1] << std::endl;
      std::vector<SocketPtr> connections;
      SocketPtr sock = Socket::create_unix(argv[1]);
      
      int connection_count = 0;
      while(true)
      {
        std::cout << "avaiting connection" << std::endl;
        SocketPtr con = sock->accept();
        std::cout << "got avaiting connection" << std::endl;

        connection_count += 1;

        std::cout << "receiving: " << connection_count  << std::endl;
        std::cout << ">>" << con->recv() << "<<" << std::endl;
        con->send("data received by xboxdrv\n");
      }
    }
    else if (false && argc == 3)
    {
      std::cout << "connecting to: " << argv[1] << " " << argv[2] << std::endl;
      SocketPtr sock = Socket::connect_tcp(argv[1], argv[2]);

      sock->send("GET /index.html HTTP/1.0\n\n");
      while(true)
      {
        std::cout << ">>" << sock->recv() << "<<" << std::endl;
      }
    }
    else if (argc == 3)
    {
      std::cout << "connecting to: " << argv[1] << " " << argv[2] << std::endl;
      SocketPtr sock = Socket::create_tcp(argv[1], argv[2]);

      int connection_count = 0;
      while(true)
      {
        std::cout << "awaiting connection" << std::endl;
        SocketPtr con = sock->accept();
        std::cout << "got avaiting connection" << std::endl;

        connection_count += 1;

        std::string data = con->recv();
        std::cout << "receiving: " << connection_count  << std::endl;
        std::cout << ">>" << data << "<<" << std::endl;
        con->send("data received by xboxdrv: " + data + "\n");
      }
    }
    else
    {
      std::cout << "Usage: " << argv[0] << " FILENAME" << std::endl;
      std::cout << "       " << argv[0] << " HOST PORT" << std::endl;
    }
  }
  catch(const std::exception& err)
  {
    std::cout << "error: " << err.what() << std::endl;
  }
  return 0;
}

#endif

/* EOF */
