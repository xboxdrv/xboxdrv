/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller/saitek_p3600_controller.hpp"

#include "controller_message.hpp"
#include "unpack.hpp"

namespace xboxdrv {

namespace {

// Report layout (8 bytes), bitfields low-to-high within each byte as on stable:
//   [0] dummy
//   [1] x1  [2] y1  [3] x2  [4] y2   (unsigned sticks, centre ~128)
//   [5] trigger_analog:6, x:1, a:1
//   [6] b:1, y:1, lb:1, rb:1, lt:1, rt:1, back:1, start:1
//   [7] thumb_l:1, thumb_r:1, fps:1, fps_toggle:1, dpad:4
//
// Stick bytes are converted with the same mapping the stable bitfield
// path used (signed 8-bit view + fix_int), then scaled to int16.
// Triggers share a 6-bit analog value; digital lt/rt select which side
// receives it (stable behaviour).

int fix_int(int num)
{
  if (num < 0)
  {
    return 128 + num;
  }
  else
  {
    return num - 127;
  }
}

int fix_int_6(int num)
{
  if (num < 0)
  {
    return 32 + num;
  }
  else
  {
    return num - 31;
  }
}

int get_trigger_val(bool digital, int analog)
{
  if (digital)
  {
    if (analog > 4)
    {
      return analog;
    }
    else
    {
      return 4;
    }
  }
  return 0;
}

int16_t scale_8to16(int v)
{
  // Same expansion stable used after fix_int (approx -127..127 → int16).
  if (v >= 0)
  {
    return static_cast<int16_t>(v * 32767 / 127);
  }
  else
  {
    return static_cast<int16_t>(v * 32768 / 128);
  }
}

int signed6(uint8_t data5)
{
  int raw = data5 & 0x3f;
  if (raw & 0x20)
  {
    raw -= 0x40;
  }
  return raw;
}

} // namespace

SaitekP3600Controller::SaitekP3600Controller(libusb_device* dev, bool try_detach) :
  USBController(dev),
  left_rumble(-1),
  right_rumble(-1),
  xbox(m_message_descriptor)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(1, 8);
}

SaitekP3600Controller::~SaitekP3600Controller()
{
}

void
SaitekP3600Controller::set_rumble_real(uint8_t left, uint8_t right)
{
  // not supported
}

void
SaitekP3600Controller::set_led_real(uint8_t status)
{
  // not supported
}

bool
SaitekP3600Controller::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (len != 8)
  {
    return false;
  }

  msg_out->clear();

  // data[5]: x, a
  msg_out->set_key(xbox.btn_x, unpack::bit(data + 5, 6));
  msg_out->set_key(xbox.btn_a, unpack::bit(data + 5, 7));

  // data[6]: b, y, shoulders, digital triggers, back, start
  msg_out->set_key(xbox.btn_b,     unpack::bit(data + 6, 0));
  msg_out->set_key(xbox.btn_y,     unpack::bit(data + 6, 1));
  msg_out->set_key(xbox.btn_lb,    unpack::bit(data + 6, 2));
  msg_out->set_key(xbox.btn_rb,    unpack::bit(data + 6, 3));
  msg_out->set_key(xbox.btn_lt,    unpack::bit(data + 6, 4));
  msg_out->set_key(xbox.btn_rt,    unpack::bit(data + 6, 5));
  msg_out->set_key(xbox.btn_back,  unpack::bit(data + 6, 6));
  msg_out->set_key(xbox.btn_start, unpack::bit(data + 6, 7));

  // data[7]: thumbs, fps (guide), dpad
  msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data + 7, 0));
  msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data + 7, 1));
  msg_out->set_key(xbox.btn_guide,   unpack::bit(data + 7, 2));

  int trigger_analog = fix_int_6(signed6(data[5]));
  int lt = get_trigger_val(unpack::bit(data + 6, 4), trigger_analog) * 8;
  int rt = get_trigger_val(unpack::bit(data + 6, 5), -trigger_analog) * 8;
  msg_out->set_abs(xbox.abs_lt, static_cast<int16_t>(lt), 0, 255);
  msg_out->set_abs(xbox.abs_rt, static_cast<int16_t>(rt), 0, 255);

  // Sticks: interpret byte as signed 8-bit (stable bitfield), then fix_int.
  int8_t x1 = static_cast<int8_t>(data[1]);
  int8_t y1 = static_cast<int8_t>(data[2]);
  int8_t x2 = static_cast<int8_t>(data[3]);
  int8_t y2 = static_cast<int8_t>(data[4]);

  msg_out->set_abs(xbox.abs_x1, scale_8to16(fix_int(x1)), -32768, 32767);
  msg_out->set_abs(xbox.abs_y1, scale_8to16(-fix_int(y1)), -32768, 32767);
  msg_out->set_abs(xbox.abs_x2, scale_8to16(fix_int(x2)), -32768, 32767);
  msg_out->set_abs(xbox.abs_y2, scale_8to16(-fix_int(y2)), -32768, 32767);

  switch (data[7] >> 4)
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
