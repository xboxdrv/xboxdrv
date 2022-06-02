/*
**  Xbox/Xbox360/XboxOne USB Gamepad Userspace Driver
**  Copyright (C) 2015 Andrey Turkin <andrey.turkin@gmail.com>
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

#include "controller/xboxone_wireless_controller.hpp"

#include <sstream>
#include <string.h>
#include <fmt/format.h>

#include <unsebu/usb_helper.hpp>

#include "controller_message.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"

namespace xboxdrv {

XboxOneWirelessController::XboxOneWirelessController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  dev_type(),
  endpoint_in(),
  endpoint_out(),
  m_sequence(0),
  m_battery_status(),
  m_serial(),
  xbox(m_message_descriptor)
{
  m_is_active = false;

  usb_claim_interface(0, try_detach);
  usb_submit_read(1, 32);
}

XboxOneWirelessController::~XboxOneWirelessController()
{
}

void
XboxOneWirelessController::submit_command(uint8_t cmd1, uint8_t cmd2, uint8_t const* data, int len)
{
  uint8_t cmd[4+32];
  assert(len <= 32);
  cmd[0] = cmd1;
  cmd[1] = cmd2;
  cmd[2] = m_sequence++;
  cmd[3] = static_cast<uint8_t>(len);
  memcpy(cmd+4, data, len);
  usb_write(1, cmd, len + 4);
}

void
XboxOneWirelessController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x0f, 0x00, 0x00, left, right, 0x1F, 0x00, 0x00 };
  submit_command(0x09, 0x00, rumblecmd, sizeof(rumblecmd));
}

void
XboxOneWirelessController::set_led_real(uint8_t status)
{
  // just got one measly led so...
  // FIXME map blinks to blinks, or better yet, rework API to accommodate this better
  uint8_t ledcmd[] = { 0x00, static_cast<uint8_t>(status ? 1 : 0), 30 };
  submit_command(0x0a, 0x20, ledcmd, sizeof(ledcmd));
}

bool
XboxOneWirelessController::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (len == 0)
  {
    return false;
  }
  else if (len == 32 && data[0] == 0x02 && data[1] == 0x20)
  { // Pairing request Message
    uint8_t response1[] = { 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x53 };
    uint8_t response2[] = { 0x00 };
    log_info("Pairing request");
    m_serial = "0000"; // FIXME figure out pairing request format
    submit_command(0x05, 0x20, response1, sizeof(response1));
    submit_command(0x05, 0x20, response2, sizeof(response2));
  }
  else if (len == 8 && data[0] == 0x03 && data[1] == 0x20)
  { // Battery status message
    // FIXME figure out battery message
  }
  else if (len == 6 && data[0] == 0x07 && data[1] == 0x20)
  { // Big X/guide button status message
    if (data[5] == 0x5B)
    {
// FIXME save last button state and copy it over here and in Event message;
//  better yet, make msg_out take partial updates
//      msg_out->set_key(xbox.btn_guide,   unpack::bit(data+4, 0));
//      return true;
    }
    else
    {
      log_debug("unknown: {}", raw2str(data, len));
    }
  }
  else if (len == 18 && data[0] == 0x20 && data[1] == 0x00)
  { // Event message
    uint8_t const* ptr = data+4;

    // msg.connect = unpack::bit(ptr+0, 0);
    msg_out->set_key(xbox.btn_start,   unpack::bit(ptr+0, 2));
    msg_out->set_key(xbox.btn_back,    unpack::bit(ptr+0, 3));
    msg_out->set_key(xbox.btn_a,       unpack::bit(ptr+0, 4));
    msg_out->set_key(xbox.btn_b,       unpack::bit(ptr+0, 5));
    msg_out->set_key(xbox.btn_x,       unpack::bit(ptr+0, 6));
    msg_out->set_key(xbox.btn_y,       unpack::bit(ptr+0, 7));

    msg_out->set_key(xbox.dpad_up,     unpack::bit(ptr+1, 0));
    msg_out->set_key(xbox.dpad_down,   unpack::bit(ptr+1, 1));
    msg_out->set_key(xbox.dpad_left,   unpack::bit(ptr+1, 2));
    msg_out->set_key(xbox.dpad_right,  unpack::bit(ptr+1, 3));
    msg_out->set_key(xbox.btn_lb,      unpack::bit(ptr+1, 4));
    msg_out->set_key(xbox.btn_rb,      unpack::bit(ptr+1, 5));
    msg_out->set_key(xbox.btn_thumb_l, unpack::bit(ptr+1, 6));
    msg_out->set_key(xbox.btn_thumb_r, unpack::bit(ptr+1, 7));

    msg_out->set_abs(xbox.abs_lt,      unpack::int16le(ptr+2), 0, 1023);
    msg_out->set_abs(xbox.abs_rt,      unpack::int16le(ptr+4), 0, 1023);

    msg_out->set_abs(xbox.abs_x1,      unpack::int16le(ptr+6), -32768, 32767);
    msg_out->set_abs(xbox.abs_y1,      unpack::int16le(ptr+8), -32768, 32767);

    msg_out->set_abs(xbox.abs_x2,      unpack::int16le(ptr+10), -32768, 32767);
    msg_out->set_abs(xbox.abs_y2,      unpack::int16le(ptr+12), -32768, 32767);

    return true;
  }
  else
  {
    log_debug("unknown: {}", raw2str(data, len));
  }

  return false;
}

} // namespace xboxdrv

/* EOF */
