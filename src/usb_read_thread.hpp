/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_USB_READ_THREAD_HPP
#define HEADER_USB_READ_THREAD_HPP

#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_array.hpp>
#include <list>

class USBReadThread
{
private:
  struct usb_dev_handle* m_handle;
  const int m_read_endpoint;
  const int m_read_length;

  struct Paket 
  {
    boost::shared_array<uint8_t> data;
    int length;

    Paket() :
      data(),
      length()
    {}
  };

  typedef std::list<Paket> Buffer;
  Buffer m_read_buffer;
  boost::mutex m_read_buffer_mutex;
  boost::condition m_read_buffer_cond;
  std::auto_ptr<boost::thread> m_thread;

  struct buffer_not_empty
  {
    Buffer& m_buffer;

    buffer_not_empty(Buffer& buffer)
      : m_buffer(buffer)
    {}

    bool operator()() const 
    { 
      return !m_buffer.empty(); 
    }
  };

  bool m_stop;

public:
  USBReadThread(struct usb_dev_handle* handle, int endpoint, int len);
  ~USBReadThread();

  int read(uint8_t* data, int len, int timeout);

  void start_thread();
  void stop_thread();

private:
  void run();

private:
  USBReadThread(const USBReadThread&);
  USBReadThread& operator=(const USBReadThread&);
};

#endif

/* EOF */
