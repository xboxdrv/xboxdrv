/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2026 Ingo Ruhnke <grumbel@gmail.com>
**  Copyright (C) 2014 Jan Hambrecht <jaham@gmx.net>
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

#include "controller/xeox_controller.hpp"

#include <algorithm>
#include <logmich/log.hpp>

#include "controller_message.hpp"
#include "unpack.hpp"
#include "util/string.hpp"

namespace xboxdrv {

namespace {

int16_t scale_8to16(int v)
{
  // v is already centered (byte - 0x80), range about [-128, 127]
  if (v >= 0)
  {
    return static_cast<int16_t>(v * 32767 / 127);
  }
  else
  {
    return static_cast<int16_t>(v * 32768 / 128);
  }
}

} // namespace

XeoxController::XeoxController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  m_endpoint_in(1),
  m_endpoint_out(2),
  xbox(m_message_descriptor)
{
  m_endpoint_in  = usb_find_ep(LIBUSB_ENDPOINT_IN,  LIBUSB_CLASS_HID, 0, 0);
  m_endpoint_out = usb_find_ep(LIBUSB_ENDPOINT_OUT, LIBUSB_CLASS_HID, 0, 0);

  log_debug("Xeox EP(IN):  {}", m_endpoint_in);
  log_debug("Xeox EP(OUT): {}", m_endpoint_out);

  usb_claim_interface(0, try_detach);
  usb_submit_read(m_endpoint_in, 32);
}

XeoxController::~XeoxController()
{
}

void
XeoxController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { left, right };
  usb_write(m_endpoint_out, rumblecmd, sizeof(rumblecmd));
}

void
XeoxController::set_led_real(uint8_t status)
{
  // Xeox has no host-driven player LED protocol that we know of.
  (void)status;
}

bool
XeoxController::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  // 8-byte HID report (PR #99 / hambre). Layout is device-specific, not xpad.
  if (len < 8)
  {
    log_debug("xeox unknown len={}: {}", len, raw2str(data, len));
    return false;
  }

  // Sticks: unsigned 0..255 centered at 0x80. Y inverted to match Xbox feel.
  msg_out->set_abs(xbox.abs_x1, scale_8to16(static_cast<int>(data[0]) - 0x80), -32768, 32767);
  msg_out->set_abs(xbox.abs_y1, unpack::s16_invert(scale_8to16(static_cast<int>(data[1]) - 0x80)), -32768, 32767);
  msg_out->set_abs(xbox.abs_x2, scale_8to16(static_cast<int>(data[3]) - 0x80), -32768, 32767);
  msg_out->set_abs(xbox.abs_y2, unpack::s16_invert(scale_8to16(static_cast<int>(data[2]) - 0x80)), -32768, 32767);

  // D-pad: extreme stick bytes and/or hat nibble in data[5] (0..7, 8=center typical).
  const uint8_t hat = data[5];
  msg_out->set_key(xbox.dpad_up,
                   data[1] == 0x00 || hat == 7 || hat == 0 || hat == 1);
  msg_out->set_key(xbox.dpad_down,
                   data[1] == 0xFF || hat == 3 || hat == 4 || hat == 5);
  msg_out->set_key(xbox.dpad_left,
                   data[0] == 0x00 || hat == 5 || hat == 6 || hat == 7);
  msg_out->set_key(xbox.dpad_right,
                   data[0] == 0xFF || hat == 1 || hat == 2 || hat == 3);

  // Shared analog channel for LT/RT (hardware only reports one side at a time).
  const unsigned int lt = std::min<unsigned int>(2 * (data[4] < 0x80 ? 0x80u - data[4] : 0u), 255u);
  const unsigned int rt = std::min<unsigned int>(2 * (data[4] > 0x80 ? data[4] - 0x80u : 0u), 255u);
  msg_out->set_abs(xbox.abs_lt, static_cast<int>(lt), 0, 255);
  msg_out->set_abs(xbox.abs_rt, static_cast<int>(rt), 0, 255);

  msg_out->set_key(xbox.btn_a,     unpack::bit(data + 6, 0));
  msg_out->set_key(xbox.btn_b,     unpack::bit(data + 6, 1));
  msg_out->set_key(xbox.btn_x,     unpack::bit(data + 6, 2));
  msg_out->set_key(xbox.btn_y,     unpack::bit(data + 6, 3));
  msg_out->set_key(xbox.btn_lb,    unpack::bit(data + 6, 4));
  msg_out->set_key(xbox.btn_rb,    unpack::bit(data + 6, 5));
  msg_out->set_key(xbox.btn_back,  unpack::bit(data + 6, 6));
  msg_out->set_key(xbox.btn_start, unpack::bit(data + 6, 7));

  msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data + 7, 0));
  msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data + 7, 1));

  return true;
}

} // namespace xboxdrv

/* EOF */
