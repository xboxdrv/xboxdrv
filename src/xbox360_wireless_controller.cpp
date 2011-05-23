/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xbox360_wireless_controller.hpp"

#include <sstream>
#include <boost/format.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

Xbox360WirelessController::Xbox360WirelessController(libusb_device* dev, int controller_id, 
                                                     bool try_detach) :
  USBController(dev),
  m_endpoint(),
  m_interface(),
  m_battery_status(),
  m_serial(),
  m_led_status(0)
{
  // FIXME: A little bit of a hack
  m_is_active = false;

  assert(controller_id >= 0 && controller_id < 4);
  
  // FIXME: Is hardcoding those ok?
  m_endpoint  = controller_id*2 + 1;
  m_interface = controller_id*2;

  usb_claim_interface(m_interface, try_detach);
  usb_submit_read(m_endpoint, 32);
}

Xbox360WirelessController::~Xbox360WirelessController()
{
  usb_cancel_read();
  usb_release_interface(m_interface);
}

void
Xbox360WirelessController::set_rumble(uint8_t left, uint8_t right)
{
  //                                       +-- typo? might be 0x0c, i.e. length
  //                                       v
  uint8_t rumblecmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, left, right, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_write(m_endpoint, rumblecmd, sizeof(rumblecmd));
}

void
Xbox360WirelessController::set_led(uint8_t status)
{
  m_led_status = status;
  //                                +--- Why not just status?
  //                                v
  uint8_t ledcmd[] = { 0x00, 0x00, 0x08, 0x40 + (status % 0x0e), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_write(m_endpoint, ledcmd, sizeof(ledcmd));
}

bool
Xbox360WirelessController::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == 0)
  {
    return false;
  }
  else
  {
    if (len == 2 && data[0] == 0x08)
    { // Connection Status Message
      if (data[1] == 0x00) 
      {
        log_info("connection status: nothing");

        // reset the controller into neutral position on disconnect
        memset(msg_out, 0, sizeof(*msg_out));
        set_active(false);

        return true;
      }
      else if (data[1] == 0x80) 
      {
        log_info("connection status: controller connected");
        set_led(m_led_status);
        set_active(true);
      } 
      else if (data[1] == 0x40) 
      {
        log_info("Connection status: headset connected");
      }
      else if (data[1] == 0xc0) 
      {
        log_info("Connection status: controller and headset connected");
        set_led(m_led_status);
      }
      else
      {
        log_info("Connection status: unknown");
      }
    }
    else if (len == 29)
    {
      set_active(true);

      if (data[0] == 0x00 && data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
      { // Initial Announc Message
        m_serial = (boost::format("%2x:%2x:%2x:%2x:%2x:%2x:%2x")
                  % int(data[7])
                  % int(data[8])
                  % int(data[9])
                  % int(data[10])
                  % int(data[11])
                  % int(data[12])
                  % int(data[13])).str();
        m_battery_status = data[17];
        log_info("Serial: " << m_serial);
        log_info("Battery Status: " << m_battery_status);
      }
      else if (data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0xf0 && data[4] == 0x00 && data[5] == 0x13)
      { // Event message
        msg_out->type = XBOX_MSG_XBOX360;
        Xbox360Msg& msg = msg_out->xbox360;

        uint8_t* ptr = data+4;

        msg.type   = ptr[0];
        msg.length = ptr[1];

        msg.dpad_up    = unpack::bit(ptr+2, 0);
        msg.dpad_down  = unpack::bit(ptr+2, 1);
        msg.dpad_left  = unpack::bit(ptr+2, 2);
        msg.dpad_right = unpack::bit(ptr+2, 3);

        msg.start   = unpack::bit(ptr+2, 4);
        msg.back    = unpack::bit(ptr+2, 5);
        msg.thumb_l = unpack::bit(ptr+2, 6);
        msg.thumb_r = unpack::bit(ptr+2, 7);

        msg.lb     = unpack::bit(ptr+3, 0);
        msg.rb     = unpack::bit(ptr+3, 1);
        msg.guide  = unpack::bit(ptr+3, 2);
        msg.dummy1 = unpack::bit(ptr+3, 3);

        msg.a = unpack::bit(ptr+3, 4);
        msg.b = unpack::bit(ptr+3, 5);
        msg.x = unpack::bit(ptr+3, 6);
        msg.y = unpack::bit(ptr+3, 7);

        msg.lt = ptr[4];
        msg.rt = ptr[5];

        msg.x1 = unpack::int16le(ptr+6);
        msg.y1 = unpack::int16le(ptr+8);

        msg.x2 = unpack::int16le(ptr+10);
        msg.y2 = unpack::int16le(ptr+12);

        msg.dummy2 = unpack::int32le(ptr+14);
        msg.dummy3 = unpack::int16le(ptr+18);

        return true;
      }
      else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
      { // Battery status
        m_battery_status = data[4];
        log_info("battery status: " << m_battery_status);
      }
      else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
      {
        // 0x00 0x00 0x00 0xf0 0x00 ... is send after each button
        // press, doesn't seem to contain any information
      }
      else
      {
        log_debug("unknown: " << raw2str(data, len));
      }
    }
    else
    {
      log_debug("unknown: " << raw2str(data, len));
    }
  }

  return false; 
}

/* EOF */
