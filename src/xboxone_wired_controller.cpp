/*
**  XboxOne USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
**            (C) 2015 Grzegorz Antoniak <ga@anadoxin.org>
**
**  This code is based on:
**
**  - xbox360 controller code:
**    -> xbox360_controller.cpp
**
**  - The Linux 'xpad' driver:
**    -> drivers/input/joystick/xpad.c
**
**  - XboxOne controller rev-eng work by quantus (Pekka Pöyry):
**    -> https://github.com/quantus/xbox-one-controller-protocol
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

#include "xboxone_wired_controller.hpp"

#include <sstream>
#include <boost/format.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

XboxOneWiredController::XboxOneWiredController(libusb_device* usb, int controller_id, bool try_detach): USBController(usb)
{
  endpoint  = controller_id * 2 + 1;
  interface = controller_id * 2;

  sent_auth = false;
  guide_button = false;

  usb_claim_interface(interface, try_detach);
  usb_submit_read(endpoint, 32);
}

XboxOneWiredController::~XboxOneWiredController()
{
}

bool XboxOneWiredController::parse(uint8_t* data, int len, XboxGenericMsg* omsg)
{
  if(data[0] == 0x20) {
    return parse_button_status(data, len, omsg);
  } else if(data[0] == 0x07) {
    return parse_ledbutton_status(data, len, omsg);
  } else if(data[0] == 0x03) {
    return parse_init_status(data, len, omsg);
  } else if(data[0] == 0x02) {
    return parse_auth_status(data, len, omsg);
  }

  return false;
}

void XboxOneWiredController::set_rumble_real(uint8_t left, uint8_t right)
{
    // not implemented
}

void XboxOneWiredController::set_led_real(uint8_t status)
{
    // not implemented
}

struct XboxOneButtonData
{
    uint8_t type;
    uint8_t const_0;
    uint16_t id;

    bool sync: 1;
    bool dummy1: 1;
    bool start: 1;
    bool back: 1;

    bool a: 1;
    bool b: 1;
    bool x: 1;
    bool y: 1;

    bool dpad_up: 1;
    bool dpad_down: 1;
    bool dpad_left: 1;
    bool dpad_right: 1;

    bool bumper_left: 1;
    bool bumper_right: 1;
    bool stick_left_click: 1;
    bool stick_right_click: 1;

    uint16_t trigger_left;
    uint16_t trigger_right;

    int16_t stick_left_x;
    int16_t stick_left_y;
    int16_t stick_right_x;
    int16_t stick_right_y;
};

struct XboxOneGuideData
{
    uint8_t type;
    uint8_t const_20;
    uint16_t id;

    uint8_t down;
    uint8_t dummy_const_5b;
};

bool XboxOneWiredController::parse_button_status(uint8_t* data, int len, XboxGenericMsg* omsg)
{
  omsg->type = XBOX_MSG_XBOX360;
  Xbox360Msg& msg = omsg->xbox360;
  XboxOneButtonData* button_data = (XboxOneButtonData*) data;

  memset((void*) &msg, 0, sizeof(msg));

  msg.a = button_data->a;
  msg.b = button_data->b;
  msg.x = button_data->x;
  msg.y = button_data->y;
  msg.start = button_data->start;
  msg.back = button_data->back;
  msg.dpad_up = button_data->dpad_up;
  msg.dpad_down = button_data->dpad_down;
  msg.dpad_left = button_data->dpad_left;
  msg.dpad_right = button_data->dpad_right;
  msg.lb = button_data->bumper_left;
  msg.rb = button_data->bumper_right;
  msg.thumb_l = button_data->stick_left_click;
  msg.thumb_r = button_data->stick_right_click;
  msg.lt = button_data->trigger_left / 4;
  msg.rt = button_data->trigger_right / 4;
  msg.x1 = button_data->stick_left_x;
  msg.y1 = button_data->stick_left_y;
  msg.x2 = button_data->stick_right_x;
  msg.y2 = button_data->stick_right_y;
  msg.guide = guide_button;

  return true;
}

bool XboxOneWiredController::parse_ledbutton_status(uint8_t* data, int len, XboxGenericMsg* omsg)
{
  XboxOneGuideData* gd = (XboxOneGuideData*) data;
  Xbox360Msg& msg = omsg->xbox360;

  memset((void*) &msg, 0, sizeof(msg));

  msg.type = XBOX_MSG_XBOX360;
  msg.guide = gd->down;
  guide_button = gd->down;

  return true;
}

bool XboxOneWiredController::parse_init_status(uint8_t* data, int len, XboxGenericMsg* omsg)
{
  return false;
}

bool XboxOneWiredController::parse_auth_status(uint8_t* data, int len, XboxGenericMsg* omsg)
{
  if(!sent_auth && data[1] == 0x20) {
    uint8_t authbuf[2] = { 0x05, 0x20 };
    usb_write(endpoint, authbuf, 2);
    sent_auth = true;
  }

  return false;
}
