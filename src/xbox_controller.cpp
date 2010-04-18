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

#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include "xboxmsg.hpp"
#include "xbox_controller.hpp"

XboxController::XboxController(struct usb_device* dev) :
  dev(),
  handle()

{
  handle = usb_open(dev);
  if (!handle)
    {
      throw std::runtime_error("Error opening Xbox360 controller");
    }
  else
    {
      // FIXME: bInterfaceNumber shouldn't be hardcoded
      int err = usb_claim_interface(handle, 0);
      if (err != 0) 
        {
          std::ostringstream out;
          out << "Error couldn't claim the USB interface: " << strerror(-err) << std::endl
              << "Try to run 'rmmod xpad' and start xboxdrv again.";
          throw std::runtime_error(out.str());
        }
    }
}

XboxController::~XboxController()
{
  usb_release_interface(handle, 0); 
  usb_close(handle);
}

void
XboxController::set_rumble(uint8_t left, uint8_t right)
{
  char rumblecmd[] = { 0x00, 0x06, 0x00, left, 0x00, right };
  usb_interrupt_write(handle, 2, rumblecmd, sizeof(rumblecmd), 0);
}

void
XboxController::set_led(uint8_t status)
{
  // Controller doesn't have a LED
}

bool
XboxController::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  // FIXME: Add tracking for duplicate data packages (send by logitech controller)
  uint8_t data[32];
  int ret = usb_interrupt_read(handle, 1 /*EndPoint*/, reinterpret_cast<char*>(data), sizeof(data), timeout);

  if (ret == -ETIMEDOUT)
    {
      return false;
    }
  else if (ret < 0)
    { // Error
      std::ostringstream str;
      str << "USBError: " << ret << "\n" << usb_strerror();
      throw std::runtime_error(str.str());
    }
  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
    {
      msg.type = XBOX_MSG_XBOX;
      memcpy(&msg.xbox, data, sizeof(XboxMsg));
      return true;
    }
  return false;
}

/* EOF */
