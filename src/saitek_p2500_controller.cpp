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
  usb_submit_read(1, sizeof(SaitekP2500Msg));
}

SaitekP2500Controller::~SaitekP2500Controller()
{
  usb_cancel_read();
  usb_release_interface(0);
}

void
SaitekP2500Controller::set_rumble(uint8_t left, uint8_t right)
{
  // not supported
}

void
SaitekP2500Controller::set_led(uint8_t status)
{
  // not supported
}

bool
SaitekP2500Controller::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == sizeof(SaitekP2500Msg))
  {
    SaitekP2500Msg msg_in;
    memcpy(&msg_in, data, sizeof(SaitekP2500Msg));

    memset(msg_out, 0, sizeof(*msg_out));
    msg_out->type = XBOX_MSG_XBOX360;

    msg_out->xbox360.a = msg_in.a;
    msg_out->xbox360.b = msg_in.b;
    msg_out->xbox360.x = msg_in.x;
    msg_out->xbox360.y = msg_in.y;

    msg_out->xbox360.lb = msg_in.lb;
    msg_out->xbox360.rb = msg_in.rb;

    msg_out->xbox360.lt = msg_in.lt * 255;
    msg_out->xbox360.rt = msg_in.rt * 255;

    msg_out->xbox360.start = msg_in.start;
    msg_out->xbox360.back  = msg_in.back;

    msg_out->xbox360.thumb_l = msg_in.thumb_l;
    msg_out->xbox360.thumb_r = msg_in.thumb_r;
      
    msg_out->xbox360.x1 = scale_8to16(msg_in.x1);
    msg_out->xbox360.y1 = scale_8to16(msg_in.y1);

    msg_out->xbox360.x2 = scale_8to16(msg_in.x2);
    msg_out->xbox360.y2 = scale_8to16(msg_in.y2);

    switch(msg_in.dpad)
    {
      case 0:
        msg_out->xbox360.dpad_up    = 1;
        msg_out->xbox360.dpad_down  = 0;
        msg_out->xbox360.dpad_left  = 0;
        msg_out->xbox360.dpad_right = 0;
        break;

      case 1:
        msg_out->xbox360.dpad_up    = 1;
        msg_out->xbox360.dpad_down  = 0;
        msg_out->xbox360.dpad_left  = 0;
        msg_out->xbox360.dpad_right = 1;
        break;

      case 2:
        msg_out->xbox360.dpad_up    = 0;
        msg_out->xbox360.dpad_down  = 0;
        msg_out->xbox360.dpad_left  = 0;
        msg_out->xbox360.dpad_right = 1;
        break;

      case 3:
        msg_out->xbox360.dpad_up    = 0;
        msg_out->xbox360.dpad_down  = 1;
        msg_out->xbox360.dpad_left  = 0;
        msg_out->xbox360.dpad_right = 1;
        break;

      case 4:
        msg_out->xbox360.dpad_up    = 0;
        msg_out->xbox360.dpad_down  = 1;
        msg_out->xbox360.dpad_left  = 0;
        msg_out->xbox360.dpad_right = 0;
        break;

      case 5:
        msg_out->xbox360.dpad_up    = 0;
        msg_out->xbox360.dpad_down  = 1;
        msg_out->xbox360.dpad_left  = 1;
        msg_out->xbox360.dpad_right = 0;
        break;

      case 6:
        msg_out->xbox360.dpad_up    = 0;
        msg_out->xbox360.dpad_down  = 0;
        msg_out->xbox360.dpad_left  = 1;
        msg_out->xbox360.dpad_right = 0;
        break;

      case 7:
        msg_out->xbox360.dpad_up    = 1;
        msg_out->xbox360.dpad_down  = 0;
        msg_out->xbox360.dpad_left  = 1;
        msg_out->xbox360.dpad_right = 0;
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
