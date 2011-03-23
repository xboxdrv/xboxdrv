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

#include "playstation3_usb_controller.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <string.h>

#include "log.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

Playstation3USBController::Playstation3USBController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  endpoint_in(1),
  endpoint_out(2)
{
  usb_claim_interface(0, try_detach);
  assert(!"not implemented");
}

Playstation3USBController::~Playstation3USBController()
{
  usb_release_interface(0);
}

void
Playstation3USBController::set_rumble(uint8_t left, uint8_t right)
{
  // not implemented
}

void
Playstation3USBController::set_led(uint8_t status)
{
  // not implemented
}

#define bitswap(x) x = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)

bool
Playstation3USBController::read(XboxGenericMsg& msg, int timeout)
{
  int len;
  uint8_t data[64] = {0};
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | endpoint_in, 
                                      data, sizeof(data), 
                                      &len, timeout);
  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  { // Error
    std::ostringstream str;
    str << "Playstation3USBController: USBError: " << ret << "\n" << usb_strerror(ret);
    throw std::runtime_error(str.str());
  }
  else
  {
    return parse(data, len, &msg);
  }
}

bool
Playstation3USBController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (static_cast<size_t>(len) >= sizeof(msg_out->ps3usb))
  {
    msg_out->type = XBOX_MSG_PS3USB;
    memcpy(&msg_out->ps3usb, data, sizeof(msg_out->ps3usb));

    bitswap(msg_out->ps3usb.accl_x);
    bitswap(msg_out->ps3usb.accl_y);
    bitswap(msg_out->ps3usb.accl_z);
    bitswap(msg_out->ps3usb.rot_z);

    if (false)
    {
      log_debug(boost::format("X:%5d Y:%5d Z:%5d RZ:%5d\n") 
                % (static_cast<int>(msg_out->ps3usb.accl_x) - 512) 
                % (static_cast<int>(msg_out->ps3usb.accl_y) - 512)
                % (static_cast<int>(msg_out->ps3usb.accl_z) - 512)
                % (static_cast<int>(msg_out->ps3usb.rot_z)));
    }
      
    if (false)
    {
      // values are normalized to 1g (-116 is force by gravity)
      log_debug(boost::format("X:%6.3f Y:%6.3f Z:%6.3f RZ:%6.3f\n") 
                % ((static_cast<int>(msg_out->ps3usb.accl_x) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.accl_y) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.accl_z) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.rot_z) - 5)));
    }
    
    if (false)
    {
      std::ostringstream str;
      str << len << ": ";
      for(int i = 0; i < len; ++i)
      {
        str << boost::format("%02x ") % static_cast<int>(data[i]);
      }
      str << std::endl;
      log_debug(str.str());
    }

    return true;   
  }
  else
  {
    return false;
  }
}

/* EOF */
