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

#ifndef HEADER_XBOXDRV_USB_INTERFACE_HPP
#define HEADER_XBOXDRV_USB_INTERFACE_HPP

#include <libusb.h>
#include <boost/function.hpp>
#include <map>

class USBInterface
{
private:
  libusb_device_handle* m_handle;
  int m_interface;
  typedef std::map<int, libusb_transfer*> Endpoints;
  Endpoints m_endpoints;

public:
  USBInterface(libusb_device_handle* handle, int interface);
  ~USBInterface();

  void submit_read(int endpoint, int len, 
                   const boost::function<void (uint8_t, int)>& callback);
  void cancel_read(int endpoint);

  void submit_write(int endpoint, uint8_t* data, int len);
  void cancel_write(int endpoint);

private:
  void cancel_transfer(int endpoint);

  void on_read_data(libusb_transfer *transfer);
  static void on_read_data_wrap(libusb_transfer *transfer)
  {
    static_cast<USBInterface*>(transfer->user_data)->on_read_data(transfer);
  }

  void on_write_data(libusb_transfer *transfer);
  static void on_write_data_wrap(libusb_transfer *transfer)
  {
    static_cast<USBInterface*>(transfer->user_data)->on_write_data(transfer);
  }

private:
  USBInterface(const USBInterface&);
  USBInterface& operator=(const USBInterface&);
};

#endif

/* EOF */
