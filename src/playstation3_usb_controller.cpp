/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "playstation3_usb_controller.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <string.h>

#include "log.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

Playstation3USBController::Playstation3USBController(libusb_device* dev, bool try_detach) :
  USBController(dev),
  endpoint_in(1),
  endpoint_out(2)
{
  usb_claim_interface(0, try_detach);
  usb_submit_read(endpoint_in, 64);
}

Playstation3USBController::~Playstation3USBController()
{
  usb_cancel_read();
  usb_release_interface(0);
}

void
Playstation3USBController::set_rumble_real(uint8_t left, uint8_t right)
{
  // not implemented
}

void
Playstation3USBController::set_led_real(uint8_t status)
{
  // not implemented
}

#define bitswap(x) x = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)

bool
Playstation3USBController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (static_cast<size_t>(len) >= 49)
  {
    // unsigned int unknown00 :8; // always 01
    // unsigned int unknown01 :8; // always 00

    // 02
    msg_out->set_button(XBOX_BTN_BACK,   unpack::bit(data+2, 0));
    msg_out->set_button(XBOX_BTN_THUMB_L,     unpack::bit(data+2, 1));
    msg_out->set_button(XBOX_BTN_THUMB_R,     unpack::bit(data+2, 2));
    msg_out->set_button(XBOX_BTN_START,  unpack::bit(data+2, 3));

    msg_out->set_button(XBOX_DPAD_UP,    unpack::bit(data+2, 4));
    msg_out->set_button(XBOX_DPAD_RIGHT, unpack::bit(data+2, 5));
    msg_out->set_button(XBOX_DPAD_DOWN,  unpack::bit(data+2, 6));
    msg_out->set_button(XBOX_DPAD_LEFT,  unpack::bit(data+2, 7));

    // 03
    msg_out->set_button(XBOX_BTN_LT,  unpack::bit(data+3, 0));
    msg_out->set_button(XBOX_BTN_RT,  unpack::bit(data+3, 1));
    msg_out->set_button(XBOX_BTN_LB,  unpack::bit(data+3, 2));
    msg_out->set_button(XBOX_BTN_RB,  unpack::bit(data+3, 3));

    msg_out->set_button(XBOX_BTN_Y,  unpack::bit(data+3, 4));
    msg_out->set_button(XBOX_BTN_B,  unpack::bit(data+3, 5));
    msg_out->set_button(XBOX_BTN_A,  unpack::bit(data+3, 6));
    msg_out->set_button(XBOX_BTN_X,  unpack::bit(data+3, 7));

    // 04
    msg_out->set_button(XBOX_BTN_GUIDE,  unpack::bit(data+4, 0));
    //unsigned int unknown04   :7;

    //unsigned int unknown05 :8; // always 00
    msg_out->set_axis(XBOX_AXIS_X1, data[6]);
    msg_out->set_axis(XBOX_AXIS_Y1, data[7]);
    msg_out->set_axis(XBOX_AXIS_X2, data[8]);
    msg_out->set_axis(XBOX_AXIS_X2, data[9]);

    //unsigned int unknown10 :8; // always 00
    //unsigned int unknown11 :8; // always 00
    //unsigned int unknown12 :8; // always 00
    //unsigned int unknown13 :8; // always 00

    // msg_out->set_axis(XBOX_AXIS_DPAD_UP,    data[14]);
    // msg_out->set_axis(XBOX_AXIS_DPAD_RIGHT, data[15]);
    // msg_out->set_axis(XBOX_AXIS_DPAD_DOWN,  data[16]);
    // msg_out->set_axis(XBOX_AXIS_DPAD_LEFT,  data[17]);

    // msg_out->set_button(XBOX_AXIS_L2, data[18]);
    // msg_out->set_button(XBOX_AXIS_R2, data[19]);
    // msg_out->set_button(XBOX_AXIS_L1, data[20]);
    // msg_out->set_button(XBOX_AXIS_R1, data[21]);

    msg_out->set_axis(XBOX_AXIS_Y, data[22]);
    msg_out->set_axis(XBOX_AXIS_B, data[23]);
    msg_out->set_axis(XBOX_AXIS_A, data[24]);
    msg_out->set_axis(XBOX_AXIS_X, data[25]);

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
