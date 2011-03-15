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

#include "xbox_controller.hpp"

#include <sstream>
#include <stdexcept>
#include <string.h>

#include "usb_helper.hpp"
#include "raise_exception.hpp"
#include "xboxmsg.hpp"

XboxController::XboxController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  m_endpoint_in(1),
  m_endpoint_out(2)
{
  // find endpoints
  m_endpoint_in  = usb_find_ep(LIBUSB_ENDPOINT_IN,  88, 66, 0);
  m_endpoint_out = usb_find_ep(LIBUSB_ENDPOINT_OUT, 88, 66, 0);
  
  usb_claim_interface(0, try_detach);
}

XboxController::~XboxController()
{
  usb_release_interface(0);
}

void
XboxController::set_rumble(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x06, 0x00, left, 0x00, right };
  int transferred = 0;
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_OUT | m_endpoint_out, 
                                      rumblecmd, sizeof(rumblecmd), &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

void
XboxController::set_led(uint8_t status)
{
  // Controller doesn't have a LED
}

bool
XboxController::read(XboxGenericMsg& msg, int timeout)
{
  // FIXME: Add tracking for duplicate data packages (send by logitech controller)
  uint8_t data[32];
  int len = 0;
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | m_endpoint_in,
                                      data, sizeof(data), &len, timeout);

  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
  else if (len == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg.type = XBOX_MSG_XBOX;
    memcpy(&msg.xbox, data, sizeof(XboxMsg));
    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
