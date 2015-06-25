/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller/firestorm_dual_controller.hpp"

#include <sstream>
#include <boost/format.hpp>

#include "controller_message.hpp"
#include "helper.hpp"
#include "log.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"

// 044f:b312
struct Firestorm_vsb_Msg
{
  unsigned int a   :1;
  unsigned int x   :1;
  unsigned int b   :1;
  unsigned int y   :1;

  unsigned int lb  :1;
  unsigned int lt  :1;
  unsigned int rb  :1;
  unsigned int rt  :1;

  unsigned int back  :1;
  unsigned int start :1;

  unsigned int thumb_l :1;
  unsigned int thumb_r :1;

  unsigned int dpad :4; // 0x0f == center, 0x00 == up, clockwise + 1 each

  int x1 :8;
  int y1 :8;
  int x2 :8;
  unsigned int y2 :8;
} __attribute__((__packed__));

// 044f:b304
struct FirestormMsg
{
  unsigned int a   :1;
  unsigned int x   :1;
  unsigned int b   :1;
  unsigned int y   :1;

  unsigned int lb  :1;
  unsigned int lt  :1;
  unsigned int rb  :1;
  unsigned int rt  :1;

  unsigned int back  :1;
  unsigned int start :1;

  unsigned int thumb_l :1;
  unsigned int thumb_r :1;

  unsigned int dummy :4;

  unsigned int dpad :8; // 0xf0 == center, 0x00 == up, clockwise + 10 each

  int x1 :8;
  int y1 :8;
  int x2 :8;
  unsigned int y2 :8;
} __attribute__((__packed__));

FirestormDualController::FirestormDualController(libusb_device* dev, bool is_vsb_, bool try_detach) :
  USBController(dev),
  is_vsb(is_vsb_),
  xbox(m_message_descriptor)
{
  usb_claim_interface(0, try_detach);

  if (is_vsb)
  {
    usb_submit_read(1, sizeof(Firestorm_vsb_Msg));
  }
  else
  {
    usb_submit_read(1, sizeof(FirestormMsg));
  }
}

FirestormDualController::~FirestormDualController()
{
}

void
FirestormDualController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t cmd[] = { left, right, 0x00, 0x00 };
  if (is_vsb)
  {
    usb_control(0x21, 0x09, 0x0200, 0x00, cmd, sizeof(cmd));
  }
  else
  {
    usb_control(0x21, 0x09, 0x02, 0x00, cmd, sizeof(cmd));
  }
}

void
FirestormDualController::set_led_real(uint8_t status)
{
  // not supported
}

bool
FirestormDualController::parse_vsb(const uint8_t* data_in, int len, ControllerMessage* msg_out)
{
  if (len == 6)
  {
    ControllerMessage& msg = *msg_out;

    msg.clear();

    msg.set_key(xbox.btn_a, unpack::bit(data_in, 0));
    msg.set_key(xbox.btn_b, unpack::bit(data_in, 1));
    msg.set_key(xbox.btn_x, unpack::bit(data_in, 2));
    msg.set_key(xbox.btn_y, unpack::bit(data_in, 3));

    msg.set_key(xbox.btn_lb, unpack::bit(data_in, 4));
    msg.set_key(xbox.btn_lt, unpack::bit(data_in, 5));

    msg.set_key(xbox.btn_rb, unpack::bit(data_in, 4));
    msg.set_key(xbox.btn_rt, unpack::bit(data_in, 5));


    msg.set_key(xbox.btn_start,   unpack::bit(data_in+1, 0));
    msg.set_key(xbox.btn_back,    unpack::bit(data_in+1, 1));
    msg.set_key(xbox.btn_thumb_l, unpack::bit(data_in+1, 2));
    msg.set_key(xbox.btn_thumb_r, unpack::bit(data_in+1, 3));

    // data_in.dpad == 0xf0 -> dpad centered
    // data_in.dpad == 0xe0 -> dpad-only mode is enabled

    const uint8_t dpad = static_cast<uint8_t>(data_in[1] >> 4);
    if (dpad == 0x0 || dpad == 0x7 || dpad == 0x1)
      msg.set_key(xbox.dpad_up, 1);

    if (dpad == 0x1 || dpad == 0x2 || dpad == 0x3)
      msg.set_key(xbox.dpad_right, 1);

    if (dpad == 0x3 || dpad == 0x4 || dpad == 0x5)
      msg.set_key(xbox.dpad_down, 1);

    if (dpad == 0x5 || dpad == 0x6 || dpad == 0x7)
      msg.set_key(xbox.dpad_left, 1);

    msg.set_abs(xbox.abs_x1, unpack::s8_to_s16(data_in[2]), -32768, 32767);
    msg.set_abs(xbox.abs_y1, unpack::s16_invert(unpack::s8_to_s16(data_in[3])), -32768, 32767);

    msg.set_abs(xbox.abs_x2, unpack::s8_to_s16(data_in[4]), -32768, 32767);
    msg.set_abs(xbox.abs_y2, unpack::s16_invert(unpack::s8_to_s16(data_in[5])), -32768, 32767);

    return true;
  }
  else
  {
    return false;
  }
}

bool
FirestormDualController::parse_default(const uint8_t* data_in, int len, ControllerMessage* msg_out)
{
  if (len == 7)
  {
    ControllerMessage& msg = *msg_out;

    msg.clear();

    msg.set_key(xbox.btn_a, unpack::bit(data_in, 0));
    msg.set_key(xbox.btn_b, unpack::bit(data_in, 1));
    msg.set_key(xbox.btn_x, unpack::bit(data_in, 2));
    msg.set_key(xbox.btn_y, unpack::bit(data_in, 3));

    msg.set_key(xbox.btn_lb, unpack::bit(data_in, 4));
    msg.set_key(xbox.btn_lt, unpack::bit(data_in, 5));

    msg.set_key(xbox.btn_rb, unpack::bit(data_in, 4));
    msg.set_key(xbox.btn_rt, unpack::bit(data_in, 5));


    msg.set_key(xbox.btn_start,   unpack::bit(data_in+1, 0));
    msg.set_key(xbox.btn_back,    unpack::bit(data_in+1, 1));
    msg.set_key(xbox.btn_thumb_l, unpack::bit(data_in+1, 2));
    msg.set_key(xbox.btn_thumb_r, unpack::bit(data_in+1, 3));

    // data_in.dpad == 0xf0 -> dpad centered
    // data_in.dpad == 0xe0 -> dpad-only mode is enabled

    if (data_in[2] == 0x00 || data_in[2] == 0x70 || data_in[2] == 0x10)
      msg.set_key(xbox.dpad_up, 1);

    if (data_in[2] == 0x10 || data_in[2] == 0x20 || data_in[2] == 0x30)
      msg.set_key(xbox.dpad_right, 1);

    if (data_in[2] == 0x30 || data_in[2] == 0x40 || data_in[2] == 0x50)
      msg.set_key(xbox.dpad_down, 1);

    if (data_in[2] == 0x50 || data_in[2] == 0x60 || data_in[2] == 0x70)
      msg.set_key(xbox.dpad_left, 1);

    msg.set_abs(xbox.abs_x1, unpack::s8_to_s16(data_in[2]), -32768, 32767);
    msg.set_abs(xbox.abs_y1, unpack::s16_invert(unpack::s8_to_s16(data_in[3])), -32768, 32767);

    msg.set_abs(xbox.abs_x2, unpack::s8_to_s16(data_in[4]), -32768, 32767);
    msg.set_abs(xbox.abs_y2, unpack::s16_invert(unpack::u8_to_s16(data_in[5])), -32768, 32767);

    return true;
  }
  else
  {
    return false;
  }
}

bool
FirestormDualController::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  if (is_vsb)
  {
    return parse_vsb(data, len, msg_out);
  }
  else
  {
    return parse_default(data, len, msg_out);
  }
}

/* EOF */
