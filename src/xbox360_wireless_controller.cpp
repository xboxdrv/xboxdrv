/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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
#include <ctime>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"
#include "chatpad.hpp"

Xbox360WirelessController::Xbox360WirelessController(libusb_device* dev, bool chatpad,
                                                     int controller_id, bool try_detach) :
  USBController(dev),
  m_endpoint(),
  m_interface(),
  m_battery_status(),
  m_serial(),
  m_chatpad(),
  m_chatpad_next(),
  m_chatpad_timeout(),
  m_uinput(),
  m_chatpad_lastpacket()
{
  // FIXME: A little bit of a hack
  m_is_active = false;

  assert(controller_id >= 0 && controller_id < 4);

  // FIXME: Is hardcoding those ok?
  m_endpoint  = controller_id*2 + 1;
  m_interface = controller_id*2;

  usb_claim_interface(m_interface, try_detach);
  usb_submit_read(m_endpoint, 32);

  m_chatpad = chatpad;
  if (m_chatpad)
  {
    struct input_id usbid;

    m_chatpad_timeout = 8; // chatpad would enter sleep mode in about 10s
    chatpad_send(0x1f);
    m_chatpad_next = time(NULL) + m_chatpad_timeout;
    m_chatpad_lastpacket = 0xffffffff;
    memset(m_chatpad_laststroke, 0x00, sizeof(m_chatpad_laststroke));
    memset(m_chatpad_keymap, 0x00, sizeof(m_chatpad_keymap));

    m_chatpad_keymap[CHATPAD_KEY_1] = KEY_1;
    m_chatpad_keymap[CHATPAD_KEY_2] = KEY_2;
    m_chatpad_keymap[CHATPAD_KEY_3] = KEY_3;
    m_chatpad_keymap[CHATPAD_KEY_4] = KEY_4;
    m_chatpad_keymap[CHATPAD_KEY_5] = KEY_5;
    m_chatpad_keymap[CHATPAD_KEY_6] = KEY_6;
    m_chatpad_keymap[CHATPAD_KEY_7] = KEY_7;
    m_chatpad_keymap[CHATPAD_KEY_8] = KEY_8;
    m_chatpad_keymap[CHATPAD_KEY_9] = KEY_9;
    m_chatpad_keymap[CHATPAD_KEY_0] = KEY_0;
    m_chatpad_keymap[CHATPAD_KEY_Q] = KEY_Q;
    m_chatpad_keymap[CHATPAD_KEY_W] = KEY_W;
    m_chatpad_keymap[CHATPAD_KEY_E] = KEY_E;
    m_chatpad_keymap[CHATPAD_KEY_R] = KEY_R;
    m_chatpad_keymap[CHATPAD_KEY_T] = KEY_T;
    m_chatpad_keymap[CHATPAD_KEY_Y] = KEY_Y;
    m_chatpad_keymap[CHATPAD_KEY_U] = KEY_U;
    m_chatpad_keymap[CHATPAD_KEY_I] = KEY_I;
    m_chatpad_keymap[CHATPAD_KEY_O] = KEY_O;
    m_chatpad_keymap[CHATPAD_KEY_P] = KEY_P;
    m_chatpad_keymap[CHATPAD_KEY_A] = KEY_A;
    m_chatpad_keymap[CHATPAD_KEY_S] = KEY_S;
    m_chatpad_keymap[CHATPAD_KEY_D] = KEY_D;
    m_chatpad_keymap[CHATPAD_KEY_F] = KEY_F;
    m_chatpad_keymap[CHATPAD_KEY_G] = KEY_G;
    m_chatpad_keymap[CHATPAD_KEY_H] = KEY_H;
    m_chatpad_keymap[CHATPAD_KEY_J] = KEY_J;
    m_chatpad_keymap[CHATPAD_KEY_K] = KEY_K;
    m_chatpad_keymap[CHATPAD_KEY_L] = KEY_L;
    m_chatpad_keymap[CHATPAD_KEY_COMMA] = KEY_COMMA;
    m_chatpad_keymap[CHATPAD_KEY_Z] = KEY_Z;
    m_chatpad_keymap[CHATPAD_KEY_X] = KEY_X;
    m_chatpad_keymap[CHATPAD_KEY_C] = KEY_C;
    m_chatpad_keymap[CHATPAD_KEY_V] = KEY_V;
    m_chatpad_keymap[CHATPAD_KEY_B] = KEY_B;
    m_chatpad_keymap[CHATPAD_KEY_N] = KEY_N;
    m_chatpad_keymap[CHATPAD_KEY_M] = KEY_M;
    m_chatpad_keymap[CHATPAD_KEY_PERIOD] = KEY_DOT;
    m_chatpad_keymap[CHATPAD_KEY_ENTER] = KEY_ENTER;
    m_chatpad_keymap[CHATPAD_KEY_BACKSPACE] = KEY_BACKSPACE;
    m_chatpad_keymap[CHATPAD_KEY_LEFT] = KEY_LEFT;
    m_chatpad_keymap[CHATPAD_KEY_SPACEBAR] = KEY_SPACE;
    m_chatpad_keymap[CHATPAD_KEY_RIGHT] = KEY_RIGHT;

    usbid.bustype = 0;
    usbid.vendor = 0;
    usbid.product = 0;
    usbid.version = 0;
    m_uinput.reset(new LinuxUinput(LinuxUinput::kGenericDevice, "Xbox360 Chatpad", usbid));
    for(uint i=0; i<sizeof(m_chatpad_keymap); ++i)
    {
      if (m_chatpad_keymap[i])
        m_uinput->add_key(m_chatpad_keymap[i]);
    }
    m_uinput->add_key(KEY_LEFTSHIFT);
    m_uinput->add_key(KEY_LEFTCTRL);
    m_uinput->add_key(KEY_LEFTMETA);
    m_uinput->add_key(KEY_RIGHTALT);
    m_uinput->finish();
  }
}

Xbox360WirelessController::~Xbox360WirelessController()
{
}

void
Xbox360WirelessController::set_rumble_real(uint8_t left, uint8_t right)
{
  //                                       +-- typo? might be 0x0c, i.e. length
  //                                       v
  uint8_t rumblecmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, left, right, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_write(m_endpoint, rumblecmd, sizeof(rumblecmd));
}

void
Xbox360WirelessController::set_led_real(uint8_t status)
{
  //                                +--- Why not just status?
  //                                v
  uint8_t ledcmd[] = { 0x00, 0x00, 0x08, static_cast<uint8_t>(0x40 + (status % 0x0e)), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
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
        set_led_real(get_led());
        set_active(true);
      }
      else if (data[1] == 0x40)
      {
        log_info("Connection status: headset connected");
      }
      else if (data[1] == 0xc0)
      {
        log_info("Connection status: controller and headset connected");
        set_led_real(get_led());
      }
      else
      {
        log_info("Connection status: unknown");
      }

      // Connection status other than nothing
      if (m_chatpad)
      {
        chatpad_send(0x1f);
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

        if (m_chatpad && time(NULL) > m_chatpad_next)
        { // wake up chatpad if timeout
          chatpad_send(0x1f);
          m_chatpad_next = time(NULL) + m_chatpad_timeout;
        }

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
      else if (data[0] == 0x00 && data[1] == 0x02 && data[2] == 0x00 && data[3] == 0xf0)
      { // Chatpad
        if (m_chatpad && m_chatpad_lastpacket != static_cast<uint32_t>(data[24] << 24 | data[25] << 16 | data[26] << 24 | data[27]))
        {
          log_debug("chatpad: " << raw2str(data+24, 5));
          m_chatpad_lastpacket = static_cast<uint32_t>(data[24] << 24 | data[25] << 16 | data[26] << 24 | data[27]); // skip dup packet

          if (data[24] == 0xf0 && data[25] == 0x03)
          { // wake up chatpad and release all keys
            chatpad_send(0x1e);
            chatpad_send(0x1b);
            if (m_chatpad_laststroke[0] & CHATPAD_MOD_SHIFT)
            {
              m_uinput->send(EV_KEY, KEY_LEFTSHIFT, 0);
            }
            if (m_chatpad_laststroke[0] & CHATPAD_MOD_GREEN)
            {
              m_uinput->send(EV_KEY, KEY_LEFTCTRL, 0);
            }
            if (m_chatpad_laststroke[0] & CHATPAD_MOD_ORANGE)
            {
              m_uinput->send(EV_KEY, KEY_RIGHTALT, 0);
            }
            if (m_chatpad_laststroke[0] & CHATPAD_MOD_PEOPLE)
            {
              m_uinput->send(EV_KEY, KEY_LEFTMETA, 0);
            }
            m_chatpad_laststroke[0] = 0;
            if (m_chatpad_laststroke[1])
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[m_chatpad_laststroke[1]], 0);
              m_chatpad_laststroke[1] = 0;
            }
            if (m_chatpad_laststroke[2])
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[m_chatpad_laststroke[2]], 0);
              m_chatpad_laststroke[2] = 0;
            }
          }
          else if (data[24] == 0xf0 && data[25] == 0x04)
          {
//             log_debug("chatpad: working");
          }
          else if (data[24] == 0x00)
          {
            uint8_t xor_modifier;

            log_debug("chatpad: keystroke " << raw2str(data+25, 4));
            xor_modifier = data[25] ^ m_chatpad_laststroke[0];
//             log_debug("chatpad: xor_modifier " << raw2str(&xor_modifier, 1));
            if (xor_modifier & CHATPAD_MOD_SHIFT) // as KEY_LEFTSHIFT
            {
              m_uinput->send(EV_KEY, KEY_LEFTSHIFT, data[25]&CHATPAD_MOD_SHIFT);
            }
            if (xor_modifier & CHATPAD_MOD_GREEN) // as KEY_LEFTCTRL
            {
              m_uinput->send(EV_KEY, KEY_LEFTCTRL, data[25]&CHATPAD_MOD_GREEN);
            }
            if (xor_modifier & CHATPAD_MOD_ORANGE) // as KEY_RIGHTALT
            {
              m_uinput->send(EV_KEY, KEY_RIGHTALT, data[25]&CHATPAD_MOD_ORANGE);
            }
            if (xor_modifier & CHATPAD_MOD_PEOPLE) // as KEY_LEFTMETA
            {
              m_uinput->send(EV_KEY, KEY_LEFTMETA, data[25]&CHATPAD_MOD_PEOPLE);
            }
            if (m_chatpad_laststroke[1] && m_chatpad_laststroke[1] != data[26]
                && m_chatpad_laststroke[1] != data[27]) // key released
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[m_chatpad_laststroke[1]], 0);
            }
            if (m_chatpad_laststroke[2] && m_chatpad_laststroke[2] != data[26]
                && m_chatpad_laststroke[1] != data[27]) // key released
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[m_chatpad_laststroke[2]], 0);
            }
            if (data[26] && data[26] != m_chatpad_laststroke[1]
                && data[26] != m_chatpad_laststroke[2]) // key pressed
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[data[26]], 1);
            }
            if (data[27] && data[27] != m_chatpad_laststroke[1]
                && data[27] != m_chatpad_laststroke[2]) // key pressed
            {
              m_uinput->send(EV_KEY, m_chatpad_keymap[data[27]], 1);
            }
            m_chatpad_laststroke[0] = data[25];
            m_chatpad_laststroke[1] = data[26];
            m_chatpad_laststroke[2] = data[27];
            m_uinput->sync();
            m_chatpad_next = time(NULL) + m_chatpad_timeout;
          }
          else
          {
            log_debug("chatpad: unknown: " << raw2str(data, len));
          }
        }
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

void
Xbox360WirelessController::chatpad_send(uint8_t cmd)
{
  uint8_t chatpadcmd[] = { 0x00, 0x00, 0x0c, cmd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  usb_write(m_endpoint, chatpadcmd, sizeof(chatpadcmd));
}

/* EOF */
