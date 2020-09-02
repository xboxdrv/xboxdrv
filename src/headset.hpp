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

#ifndef HEADER_XBOXDRV_HEADSET_HPP
#define HEADER_XBOXDRV_HEADSET_HPP

#include <libusb.h>
#include <memory>
#include <string>

#include "usb_interface.hpp"

class Headset
{
private:
  libusb_device_handle* m_handle;
  std::unique_ptr<USBInterface> m_interface;

  std::unique_ptr<std::ofstream> m_fout;
  std::unique_ptr<std::ifstream> m_fin;

public:
  Headset(libusb_device_handle* handle, bool debug);
  ~Headset();

  void play_file(const std::string& play_filename);
  void record_file(const std::string& dump_filename);

private:
  bool send_data(libusb_transfer* transfer);
  bool receive_data(uint8_t* data, int len);

private:
  Headset(const Headset&);
  Headset& operator=(const Headset&);
};

#endif

/* EOF */
