/*
**  Logitech Gamepad F310 driver for xboxdrv
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
**  Contributed by Doug Morse <dm@dougmorse.org>
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

#include "controller/logitech_f310_controller.hpp"

#include <sstream>

#include "controller_message.hpp"
#include "util/string.hpp"
#include "usb_helper.hpp"
#include "unpack.hpp"

LogitechF310Controller::LogitechF310Controller(libusb_device* dev, bool try_detach) :
  USBController(dev),
  left_rumble(-1),
  right_rumble(-1),
  xbox(m_message_descriptor)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(1, 32);
}

LogitechF310Controller::~LogitechF310Controller()
{
}

void
LogitechF310Controller::set_rumble_real(uint8_t left, uint8_t right)
{
  // not supported
}

void
LogitechF310Controller::set_led_real(uint8_t status)
{
  // not supported
}

bool
LogitechF310Controller::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  if (len == 20)
  {
    msg_out->clear();

    msg_out->set_key(xbox.btn_a, unpack::bit(data+3, 4));
    msg_out->set_key(xbox.btn_b, unpack::bit(data+3, 5));
    msg_out->set_key(xbox.btn_x, unpack::bit(data+3, 6));
    msg_out->set_key(xbox.btn_y, unpack::bit(data+3, 7));

    msg_out->set_key(xbox.btn_rb, unpack::bit(data+3, 1));
    msg_out->set_key(xbox.btn_lb, unpack::bit(data+3, 0));

    msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data+2, 7));
    msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data+2, 6));
    msg_out->set_key(xbox.btn_start, unpack::bit(data+2, 4));
    msg_out->set_key(xbox.btn_back,  unpack::bit(data+2, 5));

    msg_out->set_abs(xbox.abs_lt, data[4], 0, 255);
    msg_out->set_abs(xbox.abs_rt, data[5], 0, 255);

    msg_out->set_key(xbox.dpad_up,    unpack::bit(data+2, 0));
    msg_out->set_key(xbox.dpad_down,  unpack::bit(data+2, 1));
    msg_out->set_key(xbox.dpad_left,  unpack::bit(data+2, 2));
    msg_out->set_key(xbox.dpad_right, unpack::bit(data+2, 3));

    // the logitech gamepad f310 reports its two joysticks in a rather
    // odd manner: only 8 bits are used with one bit -- the most significant
    // bit (MSB) -- reserved to indicate direction.  this makes all four of
    // these axes asymmetrical.  for example, the raw data for x1 ranges
    // from 0-127 for far left to almost centered, 128 for centered, and
    // 1-127 for almost centered to far right.  thus, to obtain constant
    // linear scaling, different scaling factors are used on each half axis
    // because of this asymmetry.

    // it is worth noting that the information at byte offset 7 is
    // *completely* redundant with that of byte offset of 6.  the same holds
    // for bytes offsets 9, 11, and 13 in regards to byte offsets 8, 10,
    // and 12, respectively.  this might make it seem like one could just
    // use all 16 bits and use helper functions like s16_invert and
    // and unpack::int16le in order to define the axes.  doing so is
    // problematic, however, because of the way that logitech manipulates
    // the MSB for each byte of the byte pair.  ultimately, only 7 bits
    // of information are availble for each half axis, so the code below
    // makes the most of it.

    msg_out->set_abs(xbox.abs_x1, unpack::u8_to_s16(data[6]), -32768, 32767);
    msg_out->set_abs(xbox.abs_y1, unpack::s16_invert(unpack::u8_to_s16(data[8])), -32768, 32767);

    msg_out->set_abs(xbox.abs_x2, unpack::u8_to_s16(data[10]), -32768, 32767);
    msg_out->set_abs(xbox.abs_y2, unpack::s16_invert(unpack::u8_to_s16(data[12])), -32768, 32767);

    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
