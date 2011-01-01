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

#ifndef HEADER_XBOXDRV_CHATPAD_HPP
#define HEADER_XBOXDRV_CHATPAD_HPP

#include <stdint.h>
#include <memory>
#include <boost/thread.hpp>

class LinuxUinput;

enum {
  CHATPAD_KEY_1 = 0x17,
  CHATPAD_KEY_2 = 0x16,
  CHATPAD_KEY_3 = 0x15,
  CHATPAD_KEY_4 = 0x14,
  CHATPAD_KEY_5 = 0x13,
  CHATPAD_KEY_6 = 0x12,
  CHATPAD_KEY_7 = 0x11,
  CHATPAD_KEY_8 = 0x67,
  CHATPAD_KEY_9 = 0x66,
  CHATPAD_KEY_0 = 0x65,
  CHATPAD_KEY_Q = 0x27,
  CHATPAD_KEY_W = 0x26,
  CHATPAD_KEY_E = 0x25,
  CHATPAD_KEY_R = 0x24,
  CHATPAD_KEY_T = 0x23,
  CHATPAD_KEY_Y = 0x22,
  CHATPAD_KEY_U = 0x21,
  CHATPAD_KEY_I = 0x76,
  CHATPAD_KEY_O = 0x75,
  CHATPAD_KEY_P = 0x64,
  CHATPAD_KEY_A = 0x37,
  CHATPAD_KEY_S = 0x36,
  CHATPAD_KEY_D = 0x35,
  CHATPAD_KEY_F = 0x34,
  CHATPAD_KEY_G = 0x33,
  CHATPAD_KEY_H = 0x32,
  CHATPAD_KEY_J = 0x31,
  CHATPAD_KEY_K = 0x77,
  CHATPAD_KEY_L = 0x72,
  CHATPAD_KEY_COMMA = 0x62,
  CHATPAD_KEY_Z = 0x46,
  CHATPAD_KEY_X = 0x45,
  CHATPAD_KEY_C = 0x44,
  CHATPAD_KEY_V = 0x43,
  CHATPAD_KEY_B = 0x42,
  CHATPAD_KEY_N = 0x41,
  CHATPAD_KEY_M = 0x52,
  CHATPAD_KEY_PERIOD    = 0x53,
  CHATPAD_KEY_ENTER     = 0x63,
  CHATPAD_KEY_BACKSPACE = 0x71,
  CHATPAD_KEY_LEFT      = 0x55,
  CHATPAD_KEY_SPACEBAR  = 0x54,
  CHATPAD_KEY_RIGHT     = 0x51,

  CHATPAD_MOD_SHIFT  = 0x01,
  CHATPAD_MOD_GREEN  = 0x02,
  CHATPAD_MOD_ORANGE = 0x04,
  CHATPAD_MOD_PEOPLE = 0x08
};

class Chatpad
{
private:
  struct ChatpadMsg
  {
    uint8_t type;
    
    union {
      struct {
        uint8_t unknown1;
        uint8_t unknown2;
        uint8_t count1;
        uint8_t count2;
      } __attribute__((__packed__)) clock;

      struct {
        uint8_t zero1;
        uint8_t modifier;
        uint8_t scancode1;
        uint8_t scancode2;
        uint8_t zero3;
      } __attribute__((__packed__)) key;
    };
  } __attribute__((__packed__));

  struct ChatpadKeyMsg {
    uint8_t zero1;
    uint8_t modifier;
    uint8_t scancode1;
    uint8_t scancode2;
    uint8_t zero3;
  } __attribute__((__packed__));

private:
  struct usb_dev_handle* m_handle;
  bool m_quit_thread;
  std::auto_ptr<boost::thread> m_read_thread;
  std::auto_ptr<boost::thread> m_keep_alive_thread;
  std::auto_ptr<LinuxUinput> m_uinput;
  int m_keymap[256];
  bool m_state[256];

public:
  Chatpad(struct usb_dev_handle* handle);
  ~Chatpad();

  void send_init();
  void start_threads();

  void process(const ChatpadKeyMsg& msg);
  void init_uinput();

private:
  void read_thread();
  void keep_alive_thread();

private:
  Chatpad(const Chatpad&);
  Chatpad& operator=(const Chatpad&);
};

#endif

/* EOF */
