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

#ifndef HEADER_XBOXDRV_USB_INTERFACE_HPP
#define HEADER_XBOXDRV_USB_INTERFACE_HPP

#include <libusb.h>
#include <functional>
#include <map>

struct USBReadCallback;
struct USBWriteCallback;

class USBInterface
{
private:
  libusb_device_handle* m_handle;
  int m_interface;
  typedef std::map<int, libusb_transfer*> Endpoints;
  Endpoints m_endpoints;

public:
  USBInterface(libusb_device_handle* handle, int interface, bool try_detach = false);
  ~USBInterface();

  void submit_read(int endpoint, int len,
                   const std::function<bool (uint8_t*, int)>& callback);
  void cancel_read(int endpoint);

  // FIXME: could add a prepare_write() that does what submit_write()
  // does, but uses the callback to fill the data instead of getting
  // it as argument
  void submit_write(int endpoint, uint8_t* data, int len,
                    const std::function<bool (libusb_transfer*)>& callback);
  void cancel_write(int endpoint);

private:
  void cancel_transfer(int endpoint);

  void on_read_data(USBReadCallback* callback, libusb_transfer *transfer);
  void on_write_data(USBWriteCallback* callback, libusb_transfer *transfer);

private:
  static void on_read_data_wrap(libusb_transfer *transfer);
  static void on_write_data_wrap(libusb_transfer *transfer);

private:
  USBInterface(const USBInterface&);
  USBInterface& operator=(const USBInterface&);
};

#endif

/* EOF */
