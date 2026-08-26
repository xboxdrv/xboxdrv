/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
**  Copyright (C) 2016 James Le Cuirot <chewi@gentoo.org>
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

#include "controller/t_wireless_controller.hpp"

#include <cstring>

#include <unsebu/usb_helper.hpp>

#include "controller_message.hpp"
#include "unpack.hpp"

namespace xboxdrv {

namespace {

// Report layout from PR #206 (19 bytes). Many analog channels exist but only
// sticks + triggers are mapped; face/shoulder buttons are digital.
struct TWirelessMsg
{
  // data[0]
  unsigned int b1 :1;
  unsigned int b2 :1;
  unsigned int b3 :1;
  unsigned int b4 :1;
  unsigned int b5 :1;
  unsigned int b6 :1;
  unsigned int b7 :1; // digital L2
  unsigned int b8 :1; // digital R2

  // data[1]
  unsigned int select :1;
  unsigned int start  :1;
  unsigned int thumb_l :1;
  unsigned int thumb_r :1;
  unsigned int home :1;
  unsigned int :3;

  // data[2]
  unsigned int dpad :4;
  unsigned int :4;

  unsigned int x1 :8; // data[3]
  unsigned int y1 :8; // data[4]
  unsigned int x2 :8; // data[5]
  unsigned int y2 :8; // data[6]

  // data[7-16] unused extra analogs
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

static_assert(sizeof(TWirelessMsg) == 19, "TWirelessMsg size");

} // namespace

TWirelessController::TWirelessController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  xbox(m_message_descriptor)
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
  (void)status;
}

bool
TWirelessController::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (len != static_cast<int>(sizeof(TWirelessMsg)))
  {
    return false;
  }

  TWirelessMsg msg_in;
  std::memcpy(&msg_in, data, sizeof(TWirelessMsg));

  msg_out->clear();

  msg_out->set_key(xbox.btn_a, msg_in.b2);
  msg_out->set_key(xbox.btn_b, msg_in.b3);
  msg_out->set_key(xbox.btn_x, msg_in.b1);
  msg_out->set_key(xbox.btn_y, msg_in.b4);

  msg_out->set_key(xbox.btn_lb, msg_in.b5);
  msg_out->set_key(xbox.btn_rb, msg_in.b6);

  msg_out->set_abs(xbox.abs_lt, static_cast<int>(msg_in.l2), 0, 255);
  msg_out->set_abs(xbox.abs_rt, static_cast<int>(msg_in.r2), 0, 255);

  msg_out->set_key(xbox.btn_start, msg_in.start);
  msg_out->set_key(xbox.btn_back, msg_in.select);
  msg_out->set_key(xbox.btn_guide, msg_in.home);

  msg_out->set_key(xbox.btn_thumb_l, msg_in.thumb_l);
  msg_out->set_key(xbox.btn_thumb_r, msg_in.thumb_r);

  // Sticks: unsigned 0..255; Y inverted to match Xbox feel (PR #206).
  msg_out->set_abs(xbox.abs_x1, unpack::u8_to_s16(static_cast<uint8_t>(msg_in.x1)), -32768, 32767);
  msg_out->set_abs(xbox.abs_y1, unpack::s16_invert(unpack::u8_to_s16(static_cast<uint8_t>(msg_in.y1))), -32768, 32767);
  msg_out->set_abs(xbox.abs_x2, unpack::u8_to_s16(static_cast<uint8_t>(msg_in.x2)), -32768, 32767);
  msg_out->set_abs(xbox.abs_y2, unpack::s16_invert(unpack::u8_to_s16(static_cast<uint8_t>(msg_in.y2))), -32768, 32767);

  switch (msg_in.dpad)
  {
    case 0:
      msg_out->set_key(xbox.dpad_up, 1);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
    case 1:
      msg_out->set_key(xbox.dpad_up, 1);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 1);
      break;
    case 2:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 1);
      break;
    case 3:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 1);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 1);
      break;
    case 4:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 1);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
    case 5:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 1);
      msg_out->set_key(xbox.dpad_left, 1);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
    case 6:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 1);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
    case 7:
      msg_out->set_key(xbox.dpad_up, 1);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 1);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
    default:
      msg_out->set_key(xbox.dpad_up, 0);
      msg_out->set_key(xbox.dpad_down, 0);
      msg_out->set_key(xbox.dpad_left, 0);
      msg_out->set_key(xbox.dpad_right, 0);
      break;
  }

  return true;
}

} // namespace xboxdrv

/* EOF */
