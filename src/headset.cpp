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

#include <fstream>
#include <errno.h>
#include <boost/bind.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"

Headset::Headset(libusb_device_handle* handle, 
                 bool debug) :
  m_handle(handle),
  m_interface(new USBInterface(m_handle, 1)),
  m_fout(),
  m_fin()
{
}

Headset::~Headset()
{
  m_interface.reset();
}

void
Headset::play_file(const std::string& filename)
{
  m_fin.reset(new std::ifstream(filename.c_str(), std::ios::binary));

  if (!*m_fin)
  {
    std::ostringstream out;
    out << "[headset] " << filename << ": " << strerror(errno);
    throw std::runtime_error(out.str());    
  }
  else
  {
    send_data();
  }
}

void
Headset::record_file(const std::string& filename)
{
  m_fout.reset(new std::ofstream(filename.c_str(), std::ios::binary));
  
  if (!*m_fout)
  {
    raise_exception(std::runtime_error, filename << ": " << strerror(errno));
  }
  else
  {
    //FIXME:m_interface->submit_read(3, 32);
  }
}

void
Headset::send_data()
{
  uint8_t data[32];
  int len = m_fin->read(reinterpret_cast<char*>(data), sizeof(data)).gcount();
  
  if (len != 32)
  {
    log_error("short read");
  }
  else
  {
    //FIXME: m_interface->submit_write(data, 32,
    //                          boost::bind(&Headset::send_data, this));
  }
}

void
Headset::read_data(uint8_t* data, int len)
{
  if (m_fout.get())
  {
    m_fout->write(reinterpret_cast<char*>(data), len);
  }
  log_debug(raw2str(data, len));

  //FIXME: m_interface->submit_read(3, 32,
  //                           boost::bind(&Headset::send_data, this, _1, _2));
}

/* EOF */
