/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "t_wireless_controller.hpp"

#include <sstream>

#include "helper.hpp"
#include "usb_helper.hpp"

struct TWirelessMsg
{
  // data[0];
  unsigned int b1 :1;
  unsigned int b2 :1;
  unsigned int b3 :1;
  unsigned int b4 :1;
  unsigned int b5 :1;
  unsigned int b6 :1;
  unsigned int b7 :1; // Digital l2
  unsigned int b8 :1; // Digital r2

  // data[1];
  unsigned int select  :1;
  unsigned int start   :1;
  unsigned int thumb_l :1;
  unsigned int thumb_r :1;
  unsigned int home    :1;
  unsigned int         :3;

  // data[2];
  unsigned int dpad :4;
  unsigned int      :4;

  unsigned int x1 :8; // data[3]
  unsigned int y1 :8; // data[4]

  unsigned int x2 :8; // data[5]
  unsigned int y2 :8; // data[6]

  // data[7-16]
  // Unused analogs
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;
  unsigned int :8;

  unsigned int l2 :8; // data[17]
  unsigned int r2 :8; // data[18]

} __attribute__((__packed__));

TWirelessController::TWirelessController(libusb_device* dev, bool try_detach) :
  USBController(dev)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(1, sizeof(TWirelessMsg));
}

TWirelessController::~TWirelessController()
{
}

void
TWirelessController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t cmd[] = { left, right, 0x00, 0x00 };
  usb_control(0x21, 0x09, 0x0200, 0x00, cmd, sizeof(cmd));
}

void
TWirelessController::set_led_real(uint8_t status)
{
  // not supported
}

bool
TWirelessController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == sizeof(TWirelessMsg))
  {
    TWirelessMsg msg_in;
    memcpy(&msg_in, data, sizeof(TWirelessMsg));

    memset(msg_out, 0, sizeof(*msg_out));
    msg_out->type = XBOX_MSG_XBOX360;

    msg_out->xbox360.a = msg_in.b2;
    msg_out->xbox360.b = msg_in.b3;
    msg_out->xbox360.x = msg_in.b1;
    msg_out->xbox360.y = msg_in.b4;

    msg_out->xbox360.lb = msg_in.b5;
    msg_out->xbox360.rb = msg_in.b6;

    msg_out->xbox360.lt = msg_in.l2;
    msg_out->xbox360.rt = msg_in.r2;

    msg_out->xbox360.start = msg_in.start;
    msg_out->xbox360.back  = msg_in.select;
    msg_out->xbox360.guide = msg_in.home;

    msg_out->xbox360.thumb_l = msg_in.thumb_l;
    msg_out->xbox360.thumb_r = msg_in.thumb_r;

    msg_out->xbox360.x1 = scale_x8to16(msg_in.x1);
    msg_out->xbox360.y1 = scale_y8to16(msg_in.y1);

    msg_out->xbox360.x2 = scale_x8to16(msg_in.x2);
    msg_out->xbox360.y2 = scale_y8to16(msg_in.y2);

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

inline int16_t
TWirelessController::scale_x8to16(uint8_t x)
{
  if (x > 127)
    return static_cast<int16_t>((x - 128) * 32767 / 127);
  else
    return static_cast<int16_t>((x - 127) * 32768 / 127);
}

inline int16_t
TWirelessController::scale_y8to16(uint8_t y)
{
  if (y > 127)
    return static_cast<int16_t>((128 - y) * 32768 / 127);
  else
    return static_cast<int16_t>((127 - y) * 32767 / 127);
}

/* EOF */
