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
  usb_submit_read(m_endpoint_in, 32);
}

XboxController::~XboxController()
{
}

void
XboxController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x06, 0x00, left, 0x00, right };
  usb_write(m_endpoint_out, rumblecmd, sizeof(rumblecmd));
}

void
XboxController::set_led_real(uint8_t status)
{
  // Controller doesn't have a LED
}

bool
XboxController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg_out->type = XBOX_MSG_XBOX;
    memcpy(&msg_out->xbox, data, sizeof(XboxMsg));
    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
