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

#include "controller/saitek_p2500_controller.hpp"

#include <sstream>

#include "controller_message.hpp"
#include "util/string.hpp"
#include "usb_helper.hpp"
#include "unpack.hpp"

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
  right_rumble(-1),
  xbox(m_message_descriptor)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(1, sizeof(SaitekP2500Msg));
}

SaitekP2500Controller::~SaitekP2500Controller()
{
}

void
SaitekP2500Controller::set_rumble_real(uint8_t left, uint8_t right)
{
  // not supported
}

void
SaitekP2500Controller::set_led_real(uint8_t status)
{
  // not supported
}

bool
SaitekP2500Controller::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  if (len == 7)
  {
    msg_out->clear();

    msg_out->set_key(xbox.btn_a, unpack::bit(data+5, 0));
    msg_out->set_key(xbox.btn_b, unpack::bit(data+5, 1));
    msg_out->set_key(xbox.btn_x, unpack::bit(data+5, 2));
    msg_out->set_key(xbox.btn_y, unpack::bit(data+5, 3));

    msg_out->set_key(xbox.btn_lb, unpack::bit(data+5, 4));
    msg_out->set_key(xbox.btn_lt, unpack::bit(data+5, 5));
    msg_out->set_key(xbox.btn_rb, unpack::bit(data+5, 6));
    msg_out->set_key(xbox.btn_rt, unpack::bit(data+5, 7));


    msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data+6, 0));
    msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data+6, 1));

    msg_out->set_key(xbox.btn_start, unpack::bit(data+6, 2));
    msg_out->set_key(xbox.btn_back,  unpack::bit(data+6, 3));

    msg_out->set_abs(xbox.abs_x1, unpack::s8_to_s16(data[1]), -32768, 32767);
    msg_out->set_abs(xbox.abs_y1, unpack::s8_to_s16(data[2]), -32768, 32767);

    msg_out->set_abs(xbox.abs_x2, unpack::s8_to_s16(data[3]), -32768, 32767);
    msg_out->set_abs(xbox.abs_y2, unpack::s8_to_s16(data[4]), -32768, 32767);

    switch(data[6] >> 4)
    {
      case 0:
        msg_out->set_key(xbox.dpad_up,     1);
        msg_out->set_key(xbox.dpad_down,   0);
        msg_out->set_key(xbox.dpad_left,   0);
        msg_out->set_key(xbox.dpad_right,  0);
        break;

      case 1:
        msg_out->set_key(xbox.dpad_up,     1);
        msg_out->set_key(xbox.dpad_down,   0);
        msg_out->set_key(xbox.dpad_left,   0);
        msg_out->set_key(xbox.dpad_right,  1);
        break;

      case 2:
        msg_out->set_key(xbox.dpad_up,     0);
        msg_out->set_key(xbox.dpad_down,   0);
        msg_out->set_key(xbox.dpad_left,   0);
        msg_out->set_key(xbox.dpad_right,  1);
        break;

      case 3:
        msg_out->set_key(xbox.dpad_up,     0);
        msg_out->set_key(xbox.dpad_down,   1);
        msg_out->set_key(xbox.dpad_left,   0);
        msg_out->set_key(xbox.dpad_right,  1);
        break;

      case 4:
        msg_out->set_key(xbox.dpad_up,     0);
        msg_out->set_key(xbox.dpad_down,   1);
        msg_out->set_key(xbox.dpad_left,   0);
        msg_out->set_key(xbox.dpad_right,  0);
        break;

      case 5:
        msg_out->set_key(xbox.dpad_up,     0);
        msg_out->set_key(xbox.dpad_down,   1);
        msg_out->set_key(xbox.dpad_left,   1);
        msg_out->set_key(xbox.dpad_right,  0);
        break;

      case 6:
        msg_out->set_key(xbox.dpad_up,     0);
        msg_out->set_key(xbox.dpad_down,   0);
        msg_out->set_key(xbox.dpad_left,   1);
        msg_out->set_key(xbox.dpad_right,  0);
        break;

      case 7:
        msg_out->set_key(xbox.dpad_up,     1);
        msg_out->set_key(xbox.dpad_down,   0);
        msg_out->set_key(xbox.dpad_left,   1);
        msg_out->set_key(xbox.dpad_right,  0);
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
