/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "saitek_p2500_controller.hpp"

#include <sstream>

#include "helper.hpp"
#include "usb_helper.hpp"

struct SaitekP2500Msg
{
  int dummy :8; // data[0]

  int x1 :8; // data[1]
  int y1 :8; // data[2]

  int x2 :8; // data[3]
  int y2 :8; // data[4]

  // data[5];
  unsigned int a   :1;
  unsigned int x   :1;
  unsigned int b   :1;
  unsigned int y   :1;

  unsigned int lb  :1;
  unsigned int lt  :1;
  unsigned int rb  :1;
  unsigned int rt  :1;

  // data[6];
  unsigned int thumb_l :1;
  unsigned int thumb_r :1;
 
  unsigned int start :1;
  unsigned int back  :1; // not supported
 
  unsigned int dpad :4; 
    
} __attribute__((__packed__));

SaitekP2500Controller::SaitekP2500Controller(libusb_device* dev, bool try_detach) :
  USBController(dev),
  left_rumble(-1),
  right_rumble(-1)
{
  usb_claim_interface(0, try_detach);
}

SaitekP2500Controller::~SaitekP2500Controller()
{
  usb_release_interface(0);
}

void
SaitekP2500Controller::set_rumble(uint8_t left, uint8_t right)
{
  /*
    if (left_rumble  != left ||
    right_rumble != right)
    {
    left_rumble  = left;
    right_rumble = right;

    char cmd[] = { left, right, 0x00, 0x00 };

    libusb_control_transfer(handle, 0x21, 0x09, 0x02, 0x00, cmd, sizeof(cmd), 0);
    }
  */
}

void
SaitekP2500Controller::set_led(uint8_t status)
{
  // not supported
}

bool
SaitekP2500Controller::read(XboxGenericMsg& msg, int timeout)
{
  SaitekP2500Msg data;
  int len = 0;
  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | 1,
                                      reinterpret_cast<uint8_t*>(&data), sizeof(data), 
                                      &len, timeout);
  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  { // Error
    std::ostringstream str;
    str << "USBError: " << ret << "\n" << usb_strerror(ret);
    throw std::runtime_error(str.str());
  }
  else if (len == sizeof(data))
  {
    memset(&msg, 0, sizeof(msg));
    msg.type    = XBOX_MSG_XBOX360;

    msg.xbox360.a = data.a;
    msg.xbox360.b = data.b;
    msg.xbox360.x = data.x;
    msg.xbox360.y = data.y;

    msg.xbox360.lb = data.lb;
    msg.xbox360.rb = data.rb;

    msg.xbox360.lt = data.lt * 255;
    msg.xbox360.rt = data.rt * 255;

    msg.xbox360.start = data.start;
    msg.xbox360.back  = data.back;

    msg.xbox360.thumb_l = data.thumb_l;
    msg.xbox360.thumb_r = data.thumb_r;
      
    msg.xbox360.x1 = scale_8to16(data.x1);
    msg.xbox360.y1 = scale_8to16(data.y1);

    msg.xbox360.x2 = scale_8to16(data.x2);
    msg.xbox360.y2 = scale_8to16(data.y2);

    switch(data.dpad)
    {
      case 0:
        msg.xbox360.dpad_up    = 1;
        msg.xbox360.dpad_down  = 0;
        msg.xbox360.dpad_left  = 0;
        msg.xbox360.dpad_right = 0;
        break;

      case 1:
        msg.xbox360.dpad_up    = 1;
        msg.xbox360.dpad_down  = 0;
        msg.xbox360.dpad_left  = 0;
        msg.xbox360.dpad_right = 1;
        break;

      case 2:
        msg.xbox360.dpad_up    = 0;
        msg.xbox360.dpad_down  = 0;
        msg.xbox360.dpad_left  = 0;
        msg.xbox360.dpad_right = 1;
        break;

      case 3:
        msg.xbox360.dpad_up    = 0;
        msg.xbox360.dpad_down  = 1;
        msg.xbox360.dpad_left  = 0;
        msg.xbox360.dpad_right = 1;
        break;

      case 4:
        msg.xbox360.dpad_up    = 0;
        msg.xbox360.dpad_down  = 1;
        msg.xbox360.dpad_left  = 0;
        msg.xbox360.dpad_right = 0;
        break;

      case 5:
        msg.xbox360.dpad_up    = 0;
        msg.xbox360.dpad_down  = 1;
        msg.xbox360.dpad_left  = 1;
        msg.xbox360.dpad_right = 0;
        break;

      case 6:
        msg.xbox360.dpad_up    = 0;
        msg.xbox360.dpad_down  = 0;
        msg.xbox360.dpad_left  = 1;
        msg.xbox360.dpad_right = 0;
        break;

      case 7:
        msg.xbox360.dpad_up    = 1;
        msg.xbox360.dpad_down  = 0;
        msg.xbox360.dpad_left  = 1;
        msg.xbox360.dpad_right = 0;
        break;
    }

    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
