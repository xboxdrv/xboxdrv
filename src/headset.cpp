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

#include "headset.hpp"

#include <iostream>
#include <fstream>
#include <boost/format.hpp>

Headset::Headset(struct usb_dev_handle* handle, const std::string& dump_filename) :
  m_handle(handle),
  m_quit_read_thread(false)
{
  int ret = usb_claim_interface(m_handle, 1);

  if (ret < 0)
  {
    std::ostringstream out;
    out << "[headset] " << usb_strerror();
    throw std::runtime_error(out.str());
  }

  m_read_thread.reset(new boost::thread(boost::bind(&Headset::read_thread, this, dump_filename)));
  //m_write_thread.reset(new boost::thread(boost::bind(&Chatpad::write_thread, this)));
}

Headset::~Headset()
{
  if (m_read_thread.get())
    m_read_thread->join();

  if (m_write_thread.get())
    m_write_thread->join();

  m_read_thread.release();
  m_write_thread.release();

  int err = usb_release_interface(m_handle, 1);

  if (err < 0)
  {
    std::ostringstream out;
    out << "[headset] " << usb_strerror();
    throw std::runtime_error(out.str());
  }
}

void
Headset::write_thread(const std::string& filename)
{
  //std::ifstream in(filename.c_str(), std::ios::binary);

  
}

void
Headset::read_thread(const std::string& filename)
{
  std::auto_ptr<std::ofstream> out;
  
  if (!filename.empty())
  {
    out.reset(new std::ofstream(filename.c_str(), std::ios::binary));
  
    if (!*out)
    {
      std::ostringstream out;
      out << "[headset] " << strerror(errno);
      throw std::runtime_error(out.str());
    }
  }

  while(!m_quit_read_thread)
  {
    uint8_t data[32];
    const int ret = usb_interrupt_read(m_handle, 3, reinterpret_cast<char*>(data), sizeof(data), 0);
    if (ret < 0)
    {
      std::ostringstream out;
      out << "[headset] " << usb_strerror();
      throw std::runtime_error(out.str());
    }
    else
    {
      if (ret == 0)
      {
        std::cout << "[headset] -- empty read --" << std::endl;
      }
      else
      {
        if (out.get())
        {
          out->write(reinterpret_cast<char*>(data), sizeof(data));
        }

        std::cout << "[headset] ";
        for(int i = 0; i < ret; ++i)
        {
          std::cout << boost::format("0x%02x ") % int(data[i]);
        }
        std::cout << std::endl;
      }
    }
  }
}

/* EOF */
