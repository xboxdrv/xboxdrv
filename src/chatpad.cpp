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

#include "helper.hpp"
#include "linux_uinput.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"

Chatpad::Chatpad(libusb_device_handle* handle, uint16_t bcdDevice,
                 bool no_init, bool debug) :
  m_handle(handle),
  m_bcdDevice(bcdDevice),
  m_no_init(no_init),
  m_debug(debug),
  m_quit_thread(false),
  //m_read_thread(),
  //m_keep_alive_thread(),
  m_uinput(),
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
  //m_read_thread->join();
  //m_keep_alive_thread->join();
}

void
Chatpad::init_uinput()
{
  struct input_id usbid;

  usbid.bustype = 0;
  usbid.vendor  = 0;
  usbid.product = 0;
  usbid.version = 0;

  m_uinput.reset(new LinuxUinput(LinuxUinput::kGenericDevice, "Xbox360 Chatpad", usbid));

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
Chatpad::send_ctrl(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
                   uint8_t* data, uint16_t length)
{
  int ret = libusb_control_transfer(m_handle, request_type, request, value, index, data, length, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_control_transfer() failed: " << usb_strerror(ret));
  }
}

void
Chatpad::set_led(unsigned int led, bool state)
{
  if (state)
  {
    m_led_state |= led;

    if (led == CHATPAD_LED_PEOPLE)
    {
      send_ctrl(0x41, 0x00, 0x000b, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_ORANGE)
    {
      send_ctrl(0x41, 0x00, 0x000a, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_GREEN)
    {
      send_ctrl(0x41, 0x00, 0x0009, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_SHIFT)
    {
      send_ctrl(0x41, 0x00, 0x0008, 0x0002, NULL, 0);
    }
  }
  else
  {
    m_led_state &= ~led;

    if (led == CHATPAD_LED_PEOPLE)
    {
      send_ctrl(0x41, 0x00, 0x0003, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_ORANGE)
    {
      send_ctrl(0x41, 0x00, 0x0002, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_GREEN)
    {
      send_ctrl(0x41, 0x00, 0x0001, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_SHIFT)
    {
      send_ctrl(0x41, 0x00, 0x0000, 0x0002, NULL, 0);
    }
    else if (led == CHATPAD_LED_BACKLIGHT)
    {
      // backlight goes on automatically, so we only provide a switch to disable it
      send_ctrl(0x41, 0x00, 0x0004, 0x0002, NULL, 0);
    }
  }
}

void
Chatpad::start_threads()
{
  //m_read_thread.reset(new boost::thread(boost::bind(&Chatpad::read_thread, this)));
  //m_keep_alive_thread.reset(new boost::thread(boost::bind(&Chatpad::keep_alive_thread, this)));
}

void
Chatpad::read_thread()
{
  try 
  {
    uint8_t data[5];
    while(!m_quit_thread)
    {
      int len = 0;
      int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | 6,
                                          data, sizeof(data), &len, 0);
      if (ret != LIBUSB_SUCCESS)
      {
        raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
      }
      else
      {
        if (g_logger.get_log_level() > Logger::kDebug)
        {
          log_debug("read: " << len << "/5: data: " << raw2str(data, len));
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
  catch(const std::exception& err)
  {
    log_error(err.what());
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
  try
  {
    // loop and send keep alives
    while(!m_quit_thread)
    {
      send_ctrl(0x41, 0x0, 0x1f, 0x02, NULL, 0);
      log_debug("0x1f");
      sleep(1);
       
      send_ctrl(0x41, 0x0, 0x1e, 0x02, NULL, 0);
      log_debug("0x1e");
      sleep(1);
    }
  }
  catch(const std::exception& err)
  {
    log_error(err.what());
  }
}

void
Chatpad::send_init()
{
  if (!m_no_init)
  {
    int ret;
    uint8_t buf[2];

    // these three will fail, but are necessary to have the later ones succeed
    ret = libusb_control_transfer(m_handle, 0x40, 0xa9, 0xa30c, 0x4423, NULL, 0, 0);
    log_debug("ret: " << usb_strerror(ret));

    ret = libusb_control_transfer(m_handle, 0x40, 0xa9, 0x2344, 0x7f03, NULL, 0, 0);
    log_debug("ret: " << usb_strerror(ret));

    ret = libusb_control_transfer(m_handle, 0x40, 0xa9, 0x5839, 0x6832, NULL, 0, 0);
    log_debug("ret: " << usb_strerror(ret));

    // make chatpad ready
    ret = libusb_control_transfer(m_handle, 0xc0, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (read 2 bytes, will return a mode)
    log_debug("ret: " << usb_strerror(ret) << " " << static_cast<int>(buf[0]) << " " << static_cast<int>(buf[1]));

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

      ret = libusb_control_transfer(m_handle, 0x40, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (send 2 bytes, data must be 0x09 0x00)
      log_debug("ret: " << usb_strerror(ret));
 
      ret = libusb_control_transfer(m_handle, 0xc0, 0xa1, 0x0000, 0xe416, buf, 2, 0); // (read 2 bytes, this should return the NEW mode)
      log_debug("ret: " << usb_strerror(ret) << " " << static_cast<int>(buf[0]) << " " << static_cast<int>(buf[1]));

      /* FIXME: not proper way to check if the chatpad is alive
         if (!(buf[1] & 2)) // FIXME: check for {9,0} for bcdDevice==0x114
         {
         throw std::runtime_error("chatpad init failure");
         }
      */
      // chatpad is enabled, so start with keep alive
    }
  }

  // only when we get "01 02" back is the chatpad ready
  libusb_control_transfer(m_handle, 0x41, 0x0, 0x1f, 0x02, 0, 0, 0);
  log_debug("0x1f");
  sleep(1);
       
  libusb_control_transfer(m_handle, 0x41, 0x0, 0x1e, 0x02, 0, 0, 0);
  log_debug("0x1e");

  // can't send 1b before 1f before one rotation
  libusb_control_transfer(m_handle, 0x41, 0x0, 0x1b, 0x02, 0, 0, 0);
  log_debug("0x1b");
}

/* EOF */
