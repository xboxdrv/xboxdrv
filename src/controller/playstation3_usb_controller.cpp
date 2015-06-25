/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "controller/playstation3_usb_controller.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <string.h>

#include "controller_message.hpp"
#include "log.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

Playstation3USBController::Playstation3USBController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  endpoint_in(1),
  endpoint_out(2),
  xbox(m_message_descriptor)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(endpoint_in, 64);
}

Playstation3USBController::~Playstation3USBController()
{
}

#define HID_GET_REPORT      0x01
#define HID_GET_IDLE        0x02
#define HID_GET_PROTOCOL    0x03
#define HID_SET_REPORT      0x09
#define HID_SET_IDLE        0x0A
#define HID_SET_PROTOCOL    0x0B

#define HID_REPORT_TYPE_INPUT   0x01
#define HID_REPORT_TYPE_OUTPUT  0x02
#define HID_REPORT_TYPE_FEATURE 0x03

void
Playstation3USBController::set_rumble_real(uint8_t left, uint8_t right)
{
  //log_tmp("Rumble: " << static_cast<int>(left) << " " << static_cast<int>(right));
  uint8_t cmd[] = {
    // FIXME: 254 isn't quite right and the right motor seems to be on/off only
    0x00, 254, right, 254, left,  // rumble values
    0x00, 0x00, 0x00, 0x00, 0x03,   // 0x10=LED1 .. 0x02=LED4
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 4
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 3
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 2
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 1
    0x00, 0x00, 0x00, 0x00, 0x00
  };

  usb_control(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, // RequestType
              HID_SET_REPORT,   // Request
              (HID_REPORT_TYPE_OUTPUT << 8) | 0x01, // Value
              0,   // Index
              cmd, sizeof(cmd));
}

void
Playstation3USBController::set_led_real(uint8_t status)
{
  //log_tmp("LEDset: " << static_cast<int>(status));

  // convert Xbox360 LED status value to PS3
  uint8_t ps3_status = 0;
  switch(status)
  {
    case 1:
      ps3_status = 0xf<<1;
      break;

    case 2:
    case 6:
      ps3_status = 0x1<<1;
      break;

    case 3:
    case 7:
      ps3_status = 0x1<<2;
      break;

    case 4:
    case 8:
      ps3_status = 0x1<<3;
      break;

    case 5:
    case 9:
      ps3_status = 0x1<<4;
      break;

    default:
      ps3_status = 0x0;
      break;
  }

  uint8_t cmd[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,   // rumble values
    0x00, 0x00, 0x00, 0x00, ps3_status, // 0x10=LED1 .. 0x02=LED4
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 4
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 3
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 2
    0xff, 0x27, 0x10, 0x00, 0x32,   // LED 1
    0x00, 0x00, 0x00, 0x00, 0x00
  };

  usb_control(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, // RequestType
              HID_SET_REPORT,   // Request
              (HID_REPORT_TYPE_OUTPUT << 8) | 0x01, // Value
              0,   // Index
              cmd, sizeof(cmd));
}

#define bitswap(x) x = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)

bool
Playstation3USBController::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  if (static_cast<size_t>(len) >= 49)
  {
    // unsigned int unknown00 :8; // always 01
    // unsigned int unknown01 :8; // always 00

    // 02
    msg_out->set_key(xbox.btn_back,    unpack::bit(data+2, 0));
    msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data+2, 1));
    msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data+2, 2));
    msg_out->set_key(xbox.btn_start,   unpack::bit(data+2, 3));

    msg_out->set_key(xbox.dpad_up,    unpack::bit(data+2, 4));
    msg_out->set_key(xbox.dpad_right, unpack::bit(data+2, 5));
    msg_out->set_key(xbox.dpad_down,  unpack::bit(data+2, 6));
    msg_out->set_key(xbox.dpad_left,  unpack::bit(data+2, 7));

    // 03
    msg_out->set_key(xbox.btn_lt,  unpack::bit(data+3, 0));
    msg_out->set_key(xbox.btn_rt,  unpack::bit(data+3, 1));
    msg_out->set_key(xbox.btn_lb,  unpack::bit(data+3, 2));
    msg_out->set_key(xbox.btn_rb,  unpack::bit(data+3, 3));

    msg_out->set_key(xbox.btn_y,  unpack::bit(data+3, 4));
    msg_out->set_key(xbox.btn_b,  unpack::bit(data+3, 5));
    msg_out->set_key(xbox.btn_a,  unpack::bit(data+3, 6));
    msg_out->set_key(xbox.btn_x,  unpack::bit(data+3, 7));

    // 04
    msg_out->set_key(xbox.btn_guide,  unpack::bit(data+4, 0));
    //unsigned int unknown04   :7;

    //unsigned int unknown05 :8; // always 00
    msg_out->set_abs(xbox.abs_x1, data[6], 0, 255);
    msg_out->set_abs(xbox.abs_y1, data[7], 0, 255);
    msg_out->set_abs(xbox.abs_x2, data[8], 0, 255);
    msg_out->set_abs(xbox.abs_x2, data[9], 0, 255);

    //unsigned int unknown10 :8; // always 00
    //unsigned int unknown11 :8; // always 00
    //unsigned int unknown12 :8; // always 00
    //unsigned int unknown13 :8; // always 00

    // msg_out->set_abs(xbox.abs_dpad_up,    data[14]);
    // msg_out->set_abs(xbox.abs_dpad_right, data[15]);
    // msg_out->set_abs(xbox.abs_dpad_down,  data[16]);
    // msg_out->set_abs(xbox.abs_dpad_left,  data[17]);

    // msg_out->set_key(xbox.abs_l2, data[18]);
    // msg_out->set_key(xbox.abs_r2, data[19]);
    // msg_out->set_key(xbox.abs_l1, data[20]);
    // msg_out->set_key(xbox.abs_r1, data[21]);

    msg_out->set_abs(xbox.abs_y, data[22], 0, 255);
    msg_out->set_abs(xbox.abs_b, data[23], 0, 255);
    msg_out->set_abs(xbox.abs_a, data[24], 0, 255);
    msg_out->set_abs(xbox.abs_x, data[25], 0, 255);

    //unsigned int unknown26 :8; // always 00
    //unsigned int unknown27 :8; // always 00
    //unsigned int unknown28 :8; // always 00

    // Bluetooth id start (or something like that)
    //unsigned int unknown29 :8;
    //unsigned int unknown30 :8;
    //unsigned int unknown31 :8;
    //unsigned int unknown32 :8;
    //unsigned int unknown33 :8;
    //unsigned int unknown34 :8;
    //unsigned int unknown35 :8;
    //unsigned int unknown36 :8;
    //unsigned int unknown37 :8;
    //unsigned int unknown38 :8;
    //unsigned int unknown39 :8;
    //unsigned int unknown40 :8;
    // Bluetooth id end

    //bitswap(msg_out->ps3usb.accl_x);
    //bitswap(msg_out->ps3usb.accl_y);
    //bitswap(msg_out->ps3usb.accl_z);
    //bitswap(msg_out->ps3usb.rot_z);

    //unsigned int accl_x :16; // little vs big endian!?!
    //unsigned int accl_y :16; // little vs big endian!?!
    //unsigned int accl_z :16; // little vs big endian!?!

    //unsigned int rot_z :16; // very low res (3 or 4 bits), neutral at 5 or 6

#if 0
    if (false)
    {
      log_debug(boost::format("X:%5d Y:%5d Z:%5d RZ:%5d\n")
                % (static_cast<int>(msg_out->ps3usb.accl_x) - 512)
                % (static_cast<int>(msg_out->ps3usb.accl_y) - 512)
                % (static_cast<int>(msg_out->ps3usb.accl_z) - 512)
                % (static_cast<int>(msg_out->ps3usb.rot_z)));
    }

    if (false)
    {
      // values are normalized to 1g (-116 is force by gravity)
      log_debug(boost::format("X:%6.3f Y:%6.3f Z:%6.3f RZ:%6.3f\n")
                % ((static_cast<int>(msg_out->ps3usb.accl_x) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.accl_y) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.accl_z) - 512) / 116.0f)
                % ((static_cast<int>(msg_out->ps3usb.rot_z) - 5)));
    }

    if (false)
    {
      std::ostringstream str;
      str << len << ": ";
      for(int i = 0; i < len; ++i)
      {
        str << boost::format("%02x ") % static_cast<int>(data[i]);
      }
      str << std::endl;
      log_debug(str.str());
    }
#endif

    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
