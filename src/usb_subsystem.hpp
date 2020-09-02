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

#ifndef HEADER_XBOXDRV_USB_SUBSYSTEM_HPP
#define HEADER_XBOXDRV_USB_SUBSYSTEM_HPP

#include <libusb.h>
#include <memory>

#include "xpad_device.hpp"

class USBGSource;
class Options;

class USBSubsystem
{
private:
  std::unique_ptr<USBGSource> m_usb_gsource;

public:
  USBSubsystem();
  ~USBSubsystem();

public:
  static void find_controller(libusb_device** dev, XPadDevice& dev_type, const Options& opts);
  static bool find_controller_by_path(const std::string& busid, const std::string& devid,
                                      libusb_device** xbox_device);
  static bool find_controller_by_id(int id, int vendor_id, int product_id, libusb_device** xbox_device);
  static bool find_xbox360_controller(int id, libusb_device** xbox_device, XPadDevice* type);

private:
  USBSubsystem(const USBSubsystem&);
  USBSubsystem& operator=(const USBSubsystem&);
};

#endif

/* EOF */
