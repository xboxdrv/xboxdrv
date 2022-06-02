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

#include "usb_subsystem.hpp"

#include <stdexcept>
#include <sstream>

#include <fmt/format.h>

#include "usb_gsource.hpp"
#include "usb_helper.hpp"

namespace xboxdrv {

USBSubsystem::USBSubsystem() :
  m_usb_gsource()
{
  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    std::ostringstream os;
    os << "libusb_init() failed: " << libusb_strerror(ret);
    throw std::runtime_error(os.str());
  }

  m_usb_gsource.reset(new USBGSource);
  m_usb_gsource->attach(NULL);
}

USBSubsystem::~USBSubsystem()
{
  m_usb_gsource.reset();
  libusb_exit(NULL);
}

} // namespace xboxdrv

/* EOF */
