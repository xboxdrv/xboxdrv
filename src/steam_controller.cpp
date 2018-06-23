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

#include "steam_controller.hpp"

#include <sstream>

#include "helper.hpp"
#include "usb_helper.hpp"
#include "unpack.hpp"

struct SteamMsg
{
  unsigned int :16;
  unsigned int status :8;
  unsigned int :8;
  unsigned int seq :16;
  unsigned int :8;

  // buttons
  unsigned int :8;
  unsigned int rt :1;
  unsigned int lt :1;
  unsigned int rb :1;
  unsigned int lb :1;
  unsigned int y :1;
  unsigned int b :1;
  unsigned int x :1;
  unsigned int a :1;
  unsigned int dpad :4;
  unsigned int back :1;
  unsigned int steam :1;
  unsigned int start :1;
  unsigned int lgrip :1;
  unsigned int rgrip :1;
  unsigned int lpad :1;
  unsigned int rpad :1;
  unsigned int lpadtouch :1;
  unsigned int rpadtouch :1;
  unsigned int :3;

  unsigned int ltrig :8;
  unsigned int rtrig :8;

  unsigned int :24;

  int lpad_x :16;
  int lpad_y :16;
  int rpad_x :16;
  int rpad_y :16;

  unsigned int :16;
  unsigned int :16;
  unsigned int :16;
  unsigned int :16;
  unsigned int :16;

  int gpitch :16;
  int groll :16;
  int gyaw :16;
  int q1 :16;
  int q2 :16;
  int q3 :16;
  int q4 :16;

  unsigned int :32;
  unsigned int :32;
  unsigned int :32;
  unsigned int :32;

} __attribute__((__packed__));

static const uint16_t interfaces[] = { 2, 1, 2, 3, 4 };
static const int endpoints[] = { 3, 2, 3, 4, 5 };

SteamController::SteamController(libusb_device* dev, uint8_t controller_id, bool try_detach) :
  USBController(dev),
  controller_id(controller_id)
{
  assert(controller_id >= 0 && controller_id < 5);
  usb_claim_interface(interfaces[controller_id], try_detach);
  usb_submit_read(endpoints[controller_id], sizeof(SteamMsg));

  uint32_t cmd[16] = {0};

  cmd[0] = unpack::swap32(0x81000000);
  send_usb_control((uint8_t*) cmd);

  cmd[0] = unpack::swap32(0x87153284);
  cmd[1] = unpack::swap32(0x03180000);
  cmd[2] = unpack::swap32(0x31020008);
  cmd[3] = unpack::swap32(0x07000707);
  cmd[4] = unpack::swap32(0x00300000);
  cmd[5] = unpack::swap32(0x2f010000);
  send_usb_control((uint8_t*) cmd);
}

SteamController::~SteamController()
{
  uint32_t cmd[16] = { unpack::swap32(0x9f046f66), unpack::swap32(0x66210000) };
  send_usb_control((uint8_t*) cmd);
}

void
SteamController::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t cmd[64] = { 0x8f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };

  cmd[2] = 0x01;
  cmd[3] = left;
  cmd[4] = left;
  send_usb_control(cmd);

  cmd[2] = 0x00;
  cmd[3] = right;
  cmd[4] = right;
  send_usb_control(cmd);
}

void
SteamController::set_led_real(uint8_t status)
{
  // not supported
}

bool
SteamController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len != sizeof(SteamMsg))
  {
    return false;
  }

  SteamMsg msg_in;
  memcpy(&msg_in, data, sizeof(SteamMsg));

  if (msg_in.status != 0x01)
  {
    return false;
  }

  memset(msg_out, 0, sizeof(*msg_out));
  msg_out->type = XBOX_MSG_XBOX360;

  msg_out->xbox360.a = msg_in.a;
  msg_out->xbox360.b = msg_in.b;
  msg_out->xbox360.x = msg_in.x;
  msg_out->xbox360.y = msg_in.y;

  msg_out->xbox360.lb = msg_in.lb;
  msg_out->xbox360.rb = msg_in.rb;

  msg_out->xbox360.lt = msg_in.ltrig;
  msg_out->xbox360.rt = msg_in.rtrig;

  msg_out->xbox360.start = msg_in.start;
  msg_out->xbox360.back  = msg_in.back;
  msg_out->xbox360.guide = msg_in.steam;

  msg_out->xbox360.thumb_l = msg_in.lpad;
  msg_out->xbox360.thumb_r = msg_in.rpad;

  msg_out->xbox360.x1 = msg_in.lpad_x;
  msg_out->xbox360.y1 = msg_in.lpad_y;

  msg_out->xbox360.x2 = msg_in.rpad_x;
  msg_out->xbox360.y2 = msg_in.rpad_y;

  switch(msg_in.dpad)
  {
    case 1:
      msg_out->xbox360.dpad_up    = 1;
      msg_out->xbox360.dpad_down  = 0;
      break;

    case 2:
      msg_out->xbox360.dpad_left  = 0;
      msg_out->xbox360.dpad_right = 1;
      break;

    case 4:
      msg_out->xbox360.dpad_left  = 1;
      msg_out->xbox360.dpad_right = 0;
      break;

    case 8:
      msg_out->xbox360.dpad_up    = 0;
      msg_out->xbox360.dpad_down  = 1;
      break;

    default:
      msg_out->xbox360.dpad_up    = 0;
      msg_out->xbox360.dpad_down  = 0;
      msg_out->xbox360.dpad_left  = 0;
      msg_out->xbox360.dpad_right = 0;
      break;
  }

  switch(msg_in.dpad)
  {
    case 1:
    case 8:
      msg_out->xbox360.dpad_left  = msg_in.lpad_x < -16384;
      msg_out->xbox360.dpad_right = msg_in.lpad_x >= 16384;
      break;
    case 2:
    case 4:
      msg_out->xbox360.dpad_up    = msg_in.lpad_y >= 16384;
      msg_out->xbox360.dpad_down  = msg_in.lpad_y < -16384;
      break;
  }

  return true;
}

void
SteamController::send_usb_control(uint8_t cmd[])
{
  usb_control(0x21, 0x09, 0x0300, interfaces[controller_id], cmd, sizeof(uint8_t) * 64);
}

/* EOF */
