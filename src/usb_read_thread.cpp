/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "usb_read_thread.hpp"

#include <string.h>

#include "log.hpp"
#include "usb_helper.hpp"

USBReadThread::USBReadThread(libusb_device_handle* handle, int endpoint, int len) : 
  m_handle(handle),
  m_read_endpoint(endpoint),
  m_read_length(len),
  m_read_buffer(),
  m_read_buffer_mutex(),
  m_read_buffer_cond(),
  m_thread(),
  m_stop(false)
{
}

USBReadThread::~USBReadThread()
{
  stop_thread();
}
  
void
USBReadThread::start_thread()
{
  assert(!m_thread.get());

  m_stop = false;
  m_thread = std::auto_ptr<boost::thread>(new boost::thread(boost::bind(&USBReadThread::run, this)));
}

void
USBReadThread::stop_thread()
{
  if (m_thread.get())
  {
    m_stop = true;
    m_thread->join();
    m_thread.reset();
  }
}

int
USBReadThread::read(uint8_t* data, int len, int* transferred, int timeout)
{
  assert(len == m_read_length);

  boost::mutex::scoped_lock lock(m_read_buffer_mutex);

  if (!m_read_buffer_cond.timed_wait(lock, boost::posix_time::milliseconds(timeout), 
                                     buffer_not_empty(m_read_buffer)))
  {
    return LIBUSB_ERROR_TIMEOUT;
  }
  else
  {
    Packet packet = m_read_buffer.front();

    *transferred = packet.length;
    memcpy(data, packet.data.get(), m_read_length);
    m_read_buffer.pop();

    return LIBUSB_SUCCESS;
  }
}

void
USBReadThread::run()
{
  boost::shared_array<uint8_t> data(new uint8_t[m_read_length]);
  
  while(!m_stop)
  {
    int len = 0;
    int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | m_read_endpoint, 
                                        reinterpret_cast<uint8_t*>(data.get()), m_read_length, 
                                        &len, 0 /*timeout*/);

    if (ret == LIBUSB_SUCCESS)
    {
      boost::mutex::scoped_lock lock(m_read_buffer_mutex);

      Packet packet;

      packet.data   = data;
      packet.length = len;

      m_read_buffer.push(packet);
          
      m_read_buffer_cond.notify_one();

      data = boost::shared_array<uint8_t>(new uint8_t[m_read_length]);
    }
    else
    {
      log_error("error while reading from USB: " << usb_strerror(ret));
      m_stop = true;
    }
  }
}

/* EOF */
