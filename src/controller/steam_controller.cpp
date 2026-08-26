/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
**  Copyright (C) 2016 James Le Cuirot <chewi@gentoo.org>
**
**  Experimental Steam Controller backend adapted from PR #222.
**  Power-off on teardown and haptics are incomplete; Xorg may still
**  see the device as a mouse until claimed. Prefer hid-steam / Steam.
*/

#include "controller/steam_controller.hpp"

#include <assert.h>
#include <string.h>

#include "controller_message.hpp"
#include "unpack.hpp"

namespace xboxdrv {

namespace {

struct SteamMsg
{
  unsigned int :16;
  unsigned int status :8;
  unsigned int :8;
  unsigned int seq :16;
  unsigned int :8;

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

// Wired uses interface/endpoint index 0; wireless dongle slots use 1..4.
static const uint16_t interfaces[] = { 2, 1, 2, 3, 4 };
static const int endpoints[] = { 3, 2, 3, 4, 5 };

} // namespace

SteamController::SteamController(libusb_device* dev, uint8_t controller_id, bool try_detach) :
  USBController(dev),
  m_controller_id(controller_id),
  xbox(m_message_descriptor)
{
  assert(controller_id < 5);
  usb_claim_interface(interfaces[controller_id], try_detach);
  usb_submit_read(endpoints[controller_id], sizeof(SteamMsg));

  // Init sequence from PR #222 / community Steam Controller userspace notes.
  uint32_t cmd[16] = {0};
  cmd[0] = unpack::swap32(0x81000000);
  send_usb_control(reinterpret_cast<uint8_t*>(cmd));

  memset(cmd, 0, sizeof(cmd));
  cmd[0] = unpack::swap32(0x87153284);
  cmd[1] = unpack::swap32(0x03180000);
  cmd[2] = unpack::swap32(0x31020008);
  cmd[3] = unpack::swap32(0x07000707);
  cmd[4] = unpack::swap32(0x00300000);
  cmd[5] = unpack::swap32(0x2f010000);
  send_usb_control(reinterpret_cast<uint8_t*>(cmd));
}

SteamController::~SteamController()
{
  // Attempt power-off / teardown; may not fully succeed (known PR #222 WIP).
  uint32_t cmd[16] = {0};
  cmd[0] = unpack::swap32(0x9f046f66);
  cmd[1] = unpack::swap32(0x66210000);
  send_usb_control(reinterpret_cast<uint8_t*>(cmd));
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
SteamController::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (len < static_cast<int>(sizeof(SteamMsg)))
  {
    return false;
  }

  SteamMsg msg_in;
  memcpy(&msg_in, data, sizeof(SteamMsg));

  if (msg_in.status != 0x01)
  {
    return false;
  }

  msg_out->clear();

  msg_out->set_key(xbox.btn_a, msg_in.a);
  msg_out->set_key(xbox.btn_b, msg_in.b);
  msg_out->set_key(xbox.btn_x, msg_in.x);
  msg_out->set_key(xbox.btn_y, msg_in.y);

  msg_out->set_key(xbox.btn_lb, msg_in.lb);
  msg_out->set_key(xbox.btn_rb, msg_in.rb);

  msg_out->set_abs(xbox.abs_lt, msg_in.ltrig, 0, 255);
  msg_out->set_abs(xbox.abs_rt, msg_in.rtrig, 0, 255);

  msg_out->set_key(xbox.btn_start, msg_in.start);
  msg_out->set_key(xbox.btn_back,  msg_in.back);
  msg_out->set_key(xbox.btn_guide, msg_in.steam);

  msg_out->set_key(xbox.btn_thumb_l, msg_in.lpad);
  msg_out->set_key(xbox.btn_thumb_r, msg_in.rpad);

  msg_out->set_abs(xbox.abs_x1, msg_in.lpad_x, -32768, 32767);
  msg_out->set_abs(xbox.abs_y1, msg_in.lpad_y, -32768, 32767);
  msg_out->set_abs(xbox.abs_x2, msg_in.rpad_x, -32768, 32767);
  msg_out->set_abs(xbox.abs_y2, msg_in.rpad_y, -32768, 32767);

  // Digital d-pad bits; left pad can also contribute when d-pad is active.
  bool du = false, dd = false, dl = false, dr = false;
  switch (msg_in.dpad)
  {
    case 1: du = true; break;
    case 2: dr = true; break;
    case 4: dl = true; break;
    case 8: dd = true; break;
    default: break;
  }

  switch (msg_in.dpad)
  {
    case 1:
    case 8:
      dl = msg_in.lpad_x < -16384;
      dr = msg_in.lpad_x >= 16384;
      break;
    case 2:
    case 4:
      du = msg_in.lpad_y >= 16384;
      dd = msg_in.lpad_y < -16384;
      break;
    default:
      break;
  }

  msg_out->set_key(xbox.dpad_up, du);
  msg_out->set_key(xbox.dpad_down, dd);
  msg_out->set_key(xbox.dpad_left, dl);
  msg_out->set_key(xbox.dpad_right, dr);

  // Grips are extra; expose as keys without default uinput map.
  // (lgrip/rgrip left unmapped to Xbox face layout.)

  return true;
}

void
SteamController::send_usb_control(uint8_t* cmd)
{
  usb_control(0x21, 0x09, 0x0300, interfaces[m_controller_id], cmd, 64);
}

} // namespace xboxdrv

/* EOF */
