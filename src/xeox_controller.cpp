/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xeox_controller.hpp"

#include "chatpad.hpp"
#include "headset.hpp"
#include "helper.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"

XeoxController::XeoxController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  dev_type(),
  endpoint_in(1),
  endpoint_out(2)
{
  // find endpoints
  //endpoint_in  = usb_find_ep(LIBUSB_ENDPOINT_IN,  LIBUSB_CLASS_VENDOR_SPEC, 93, 1);
  //endpoint_out = usb_find_ep(LIBUSB_ENDPOINT_OUT, LIBUSB_CLASS_VENDOR_SPEC, 93, 1);
  endpoint_in = 1;
  endpoint_out = 2;

  log_debug("EP(IN):  " << endpoint_in);
  log_debug("EP(OUT): " << endpoint_out);

  usb_claim_interface(0, try_detach);
  usb_submit_read(endpoint_in, 32);
}

XeoxController::~XeoxController()
{
}

void
XeoxController::set_rumble_real(uint8_t left, uint8_t right)
{
  // not implemented
}

void
XeoxController::set_led_real(uint8_t status)
{
  // not implemented
}

bool
XeoxController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == 8)
  {
    msg_out->type = XBOX_MSG_XBOX360;
    Xbox360Msg& msg = msg_out->xbox360;

    msg.x1 = scale_8to16(data[0] - 0x80);
    msg.y1 = scale_8to16(data[1] - 0x80);

    msg.x2 = scale_8to16(data[3] - 0x80);
    msg.y2 = scale_8to16(data[2] - 0x80);

    msg.dpad_up    = data[1] == 0    || data[5] == 7 || data[5] == 0 || data[5] == 1;
    msg.dpad_down  = data[1] == 0xFF || data[5] == 3 || data[5] == 4 || data[5] == 5;
    msg.dpad_left  = data[0] == 0    || data[5] == 5 || data[5] == 6 || data[5] == 7;
    msg.dpad_right = data[0] == 0xFF || data[5] == 1 || data[5] == 2 || data[5] == 3;

    msg.lt = std::min<unsigned int>(2*(data[4] < 0x80 ? 0x80 - data[4] : 0), 255);
    msg.rt = std::min<unsigned int>(2*(data[4] > 0x80 ? data[4] - 0x80 : 0), 255);

    msg.a     = unpack::bit(data+6, 0);
    msg.b     = unpack::bit(data+6, 1);
    msg.x     = unpack::bit(data+6, 2);
    msg.y     = unpack::bit(data+6, 3);
    msg.lb    = unpack::bit(data+6, 4);
    msg.rb    = unpack::bit(data+6, 5);
    msg.back  = unpack::bit(data+6, 6);
    msg.start = unpack::bit(data+6, 7);

    msg.thumb_l = unpack::bit(data+7, 0);
    msg.thumb_r = unpack::bit(data+7, 1);

    return true;
  }
  log_debug("unknown: " << raw2str(data, len));
  return false;
}

/* EOF */
