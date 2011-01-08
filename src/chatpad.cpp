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

#include "chatpad.hpp"

#include <iostream>
#include <boost/format.hpp>
#include <usb.h>
#include <string.h>

#include "linux_uinput.hpp"

Chatpad::Chatpad(struct usb_dev_handle* handle, uint16_t bcdDevice,
                 bool no_init, bool debug) :
  m_handle(handle),
  m_bcdDevice(bcdDevice),
  m_no_init(no_init),
  m_debug(debug),
  m_quit_thread(false),
  m_read_thread(),
  m_keep_alive_thread(),
  m_led_state(0)
{
  if (m_bcdDevice != 0x0110 && m_bcdDevice != 0x0114)
  {
    throw std::runtime_error("unknown bcdDevice version number, please report this issue to grumbel@gmail.com and include the output of 'lsusb -v'");
  }

  memset(m_keymap, 0, 256);
  memset(m_state, 0, 256);

  m_keymap[CHATPAD_KEY_1] = KEY_1;
  m_keymap[CHATPAD_KEY_2] = KEY_2; 
  m_keymap[CHATPAD_KEY_3] = KEY_3; 
  m_keymap[CHATPAD_KEY_4] = KEY_4; 
  m_keymap[CHATPAD_KEY_5] = KEY_5; 
  m_keymap[CHATPAD_KEY_6] = KEY_6; 
  m_keymap[CHATPAD_KEY_7] = KEY_7; 
  m_keymap[CHATPAD_KEY_8] = KEY_8; 
  m_keymap[CHATPAD_KEY_9] = KEY_9; 
  m_keymap[CHATPAD_KEY_0] = KEY_0; 
  m_keymap[CHATPAD_KEY_Q] = KEY_Q; 
  m_keymap[CHATPAD_KEY_W] = KEY_W; 
  m_keymap[CHATPAD_KEY_E] = KEY_E; 
  m_keymap[CHATPAD_KEY_R] = KEY_R; 
  m_keymap[CHATPAD_KEY_T] = KEY_T; 
  m_keymap[CHATPAD_KEY_Y] = KEY_Y; 
  m_keymap[CHATPAD_KEY_U] = KEY_U; 
  m_keymap[CHATPAD_KEY_I] = KEY_I; 
  m_keymap[CHATPAD_KEY_O] = KEY_O; 
  m_keymap[CHATPAD_KEY_P] = KEY_P; 
  m_keymap[CHATPAD_KEY_A] = KEY_A; 
  m_keymap[CHATPAD_KEY_S] = KEY_S; 
  m_keymap[CHATPAD_KEY_D] = KEY_D; 
  m_keymap[CHATPAD_KEY_F] = KEY_F; 
  m_keymap[CHATPAD_KEY_G] = KEY_G; 
  m_keymap[CHATPAD_KEY_H] = KEY_H; 
  m_keymap[CHATPAD_KEY_J] = KEY_J; 
  m_keymap[CHATPAD_KEY_K] = KEY_K; 
  m_keymap[CHATPAD_KEY_L] = KEY_L; 
  m_keymap[CHATPAD_KEY_COMMA] = KEY_COMMA; 
  m_keymap[CHATPAD_KEY_Z] = KEY_Z; 
  m_keymap[CHATPAD_KEY_X] = KEY_X; 
  m_keymap[CHATPAD_KEY_C] = KEY_C; 
  m_keymap[CHATPAD_KEY_V] = KEY_V; 
  m_keymap[CHATPAD_KEY_B] = KEY_B; 
  m_keymap[CHATPAD_KEY_N] = KEY_N; 
  m_keymap[CHATPAD_KEY_M] = KEY_M; 
  m_keymap[CHATPAD_KEY_PERIOD] = KEY_DOT;
  m_keymap[CHATPAD_KEY_ENTER] = KEY_ENTER;     
  m_keymap[CHATPAD_KEY_BACKSPACE] = KEY_BACKSPACE; 
  m_keymap[CHATPAD_KEY_LEFT] = KEY_LEFT; 
  m_keymap[CHATPAD_KEY_SPACEBAR] = KEY_SPACE;  
  m_keymap[CHATPAD_KEY_RIGHT] = KEY_RIGHT;

  m_keymap[CHATPAD_MOD_SHIFT]  = KEY_LEFTSHIFT;
  m_keymap[CHATPAD_MOD_GREEN]  = KEY_LEFTALT;
  m_keymap[CHATPAD_MOD_ORANGE] = KEY_LEFTCTRL;
  m_keymap[CHATPAD_MOD_PEOPLE] = KEY_LEFTMETA;
  
  init_uinput();
}

Chatpad::~Chatpad()
{
  m_quit_thread = true;
  m_read_thread->join();
  m_keep_alive_thread->join();
}

void
Chatpad::init_uinput()
{
  m_uinput.reset(new LinuxUinput(LinuxUinput::kGenericDevice, "Xbox360 Chatpad", 0x00, 0x00));

  for(int i = 0; i < 256; ++i)
  {
    if (m_keymap[i])
    {
      m_uinput->add_key(m_keymap[i]);
    }
  }
  m_uinput->finish();
}

bool
Chatpad::get_led(unsigned int led)
{
  return m_led_state & led;
}

void
Chatpad::set_led(unsigned int led, bool state)
{
  if (state)
  {
    m_led_state |= led;

    if (led == CHATPAD_LED_PEOPLE)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x000b, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_ORANGE)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x000a, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_GREEN)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0009, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_SHIFT)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0008, 0x0002, NULL, 0, 0);
    }
  }
  else
  {
    m_led_state &= ~led;

    if (led == CHATPAD_LED_PEOPLE)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0003, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_ORANGE)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0002, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_GREEN)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0001, 0x0002, NULL, 0, 0);
    }
    else if (led == CHATPAD_LED_SHIFT)
    {
      usb_control_msg(m_handle, 0x41, 0x00, 0x0000, 0x0002, NULL, 0, 0);
    }
  }
}

void
Chatpad::start_threads()
{
  m_read_thread.reset(new boost::thread(boost::bind(&Chatpad::read_thread, this)));
  m_keep_alive_thread.reset(new boost::thread(boost::bind(&Chatpad::keep_alive_thread, this)));
}

void
Chatpad::read_thread()
{
  uint8_t data[5];
  while(!m_quit_thread)
  {
    int len = usb_interrupt_read(m_handle, 6, reinterpret_cast<char*>(data), sizeof(data), 0);
    if (len < 0)
    {
      std::cout << "Error in read_thread" << std::endl;
      return;
    }
    else
    {
      if (m_debug)
      {
        std::cout << "[chatpad] read: " << len << "/5: data: " << std::flush;
        for(int i = 0; i < len; ++i)
        {
          std::cout << boost::format("0x%02x ") % int(data[i]);
        }
        std::cout << std::endl;
      }

      if (data[0] == 0x00)
      {
        struct ChatpadKeyMsg msg;
        memcpy(&msg, data, sizeof(msg));
        process(msg);
      }
    }
  }
}

void
Chatpad::process(const ChatpadKeyMsg& msg)
{
  // save old state
  bool old_state[256];
  memcpy(old_state, m_state, 256);

  // generate new state
  memset(m_state, 0, 256);

  m_state[CHATPAD_MOD_PEOPLE] = msg.modifier & CHATPAD_MOD_PEOPLE;
  m_state[CHATPAD_MOD_ORANGE] = msg.modifier & CHATPAD_MOD_ORANGE;
  m_state[CHATPAD_MOD_GREEN]  = msg.modifier & CHATPAD_MOD_GREEN;
  m_state[CHATPAD_MOD_SHIFT]  = msg.modifier & CHATPAD_MOD_SHIFT;

  if (msg.scancode1) m_state[msg.scancode1] = true;
  if (msg.scancode2) m_state[msg.scancode2] = true;

  // check for changes
  for(int i = 0; i < 256; ++i)
  {
    if (m_state[i] != old_state[i])
    {
      if (m_state[i])
      {
        if (i == CHATPAD_KEY_1)
        {
          usb_control_msg(m_handle, 0x41, 0x00, 0x0004, 0x0002, NULL, 0, 0);
        }

        if (i == CHATPAD_MOD_PEOPLE)
        {
          set_led(CHATPAD_LED_PEOPLE, !get_led(CHATPAD_LED_PEOPLE));
        }
        else if (i == CHATPAD_MOD_ORANGE)
        {
          set_led(CHATPAD_LED_ORANGE, !get_led(CHATPAD_LED_ORANGE));
        }
        else if (i == CHATPAD_MOD_GREEN)
        {
          set_led(CHATPAD_LED_GREEN, !get_led(CHATPAD_LED_GREEN));
        }
        else if (i == CHATPAD_MOD_SHIFT)
        {
          set_led(CHATPAD_LED_SHIFT, !get_led(CHATPAD_LED_SHIFT));
        }
      }

      m_uinput->send(EV_KEY, m_keymap[i], m_state[i]);
    }
  }
  m_uinput->sync();
}

void
Chatpad::keep_alive_thread()
{
  // loop and send keep alives
  while(!m_quit_thread)
  {
    usb_control_msg(m_handle, 0x41, 0x0, 0x1f, 0x02, 0, NULL, 0);
    if (m_debug) std::cout << "[chatpad] 0x1f" << std::endl;
    sleep(1);
       
    usb_control_msg(m_handle, 0x41, 0x0, 0x1e, 0x02, 0, NULL, 0);
    if (m_debug) std::cout << "[chatpad] 0x1e" << std::endl;
    sleep(1);
  }
}

void
Chatpad::send_init()
{
  if (!m_no_init)
  {
    int ret;
    char buf[2];

    // these three will fail, but are necessary to have the later ones succeed
    ret = usb_control_msg(m_handle, 0x40, 0xa9, 0xa30c, 0x4423, NULL, 0, 0);
    if (m_debug) std::cout << "[chatpad] ret: " << ret << std::endl;

    ret = usb_control_msg(m_handle, 0x40, 0xa9, 0x2344, 0x7f03, NULL, 0, 0);
    if (m_debug) std::cout << "[chatpad] ret: " << ret << std::endl;

    ret = usb_control_msg(m_handle, 0x40, 0xa9, 0x5839, 0x6832, NULL, 0, 0);
    if (m_debug) std::cout << "[chatpad] ret: " << ret << std::endl;

    // make chatpad ready
    ret = usb_control_msg(m_handle, 0xc0, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (read 2 bytes, will return a mode)
    if (m_debug) std::cout << "[chatpad] ret: " << ret << " " << (int)buf[0] << " " << (int)buf[1]<< std::endl;

    if (buf[1] & 2)
    {
      // chatpad already ready
      // FIXME: doesn't work
    }
    else
    {
      if (m_bcdDevice == 0x0110)
      {
        buf[0] = 0x01;
        buf[1] = 0x02;
      }
      else if (m_bcdDevice == 0x0114)
      {
        buf[0] = 0x09;
        buf[1] = 0x00;
      }
      else
      {
        assert(!"never reached");
      }

      ret = usb_control_msg(m_handle, 0x40, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (send 2 bytes, data must be 0x09 0x00)
      if (m_debug) std::cout << "[chatpad] ret: " << ret << std::endl;
 
      ret = usb_control_msg(m_handle, 0xc0, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (read 2 bytes, this should return the NEW mode)
      if (m_debug) std::cout << "[chatpad] ret: " << ret << " " << (int)buf[0] << " " << (int)buf[1]<< std::endl;

      if (!(buf[1] & 2))
      {
        throw std::runtime_error("chatpad init failure");
      }
      // chatpad is enabled, so start with keep alive
    }
  }

  // only when we get "01 02" back is the chatpad ready
  usb_control_msg(m_handle, 0x41, 0x0, 0x1f, 0x02, 0, NULL, 0);
  if (m_debug) std::cout << "[chatpad] 0x1f" << std::endl;
  sleep(1);
       
  usb_control_msg(m_handle, 0x41, 0x0, 0x1e, 0x02, 0, NULL, 0);
  if (m_debug) std::cout << "[chatpad] 0x1e" << std::endl;

  // can't send 1b before 1f before one rotation
  usb_control_msg(m_handle, 0x41, 0x0, 0x1b, 0x02, 0, NULL, 0);
  if (m_debug) std::cout << "[chatpad] 0x1b" << std::endl;
}

/* EOF */
