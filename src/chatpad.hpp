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

#ifndef HEADER_XBOXDRV_CHATPAD_HPP
#define HEADER_XBOXDRV_CHATPAD_HPP

#include <array>
#include <glib.h>
#include <libusb.h>
#include <memory>

#include <uinpp/glib_uinput.hpp>
#include <uinpp/fwd.hpp>

namespace xboxdrv {

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

enum {
  CHATPAD_LED_PEOPLE    = 1<<0,
  CHATPAD_LED_ORANGE    = 1<<1,
  CHATPAD_LED_GREEN     = 1<<2,
  CHATPAD_LED_SHIFT     = 1<<3,
  CHATPAD_LED_BACKLIGHT = 1<<4
};

enum {
  CHATPAD_LED_STATUS_PEOPLE    = (1<<0),
  CHATPAD_LED_STATUS_SHIFT     = (1<<5),
  CHATPAD_LED_STATUS_ORANGE    = (1<<4),
  CHATPAD_LED_STATUS_GREEN     = (1<<3),
  CHATPAD_LED_STATUS_BACKLIGHT = (1<<7)
};

class Chatpad
{
private:
  enum State {
    kStateInit1,
    kStateInit2,
    kStateInit3,
    kStateInit4,
    kStateInit5,
    kStateInit6,
    kStateInit_1e,
    kStateInit_1f,
    kStateInit_1b,
    kStateKeepAlive_1e,
    kStateKeepAlive_1f,
    kStateLoop
  };

  State m_init_state;

  struct ChatpadMsg
  {
    uint8_t type;

    struct ClockMsg {
      uint8_t unknown1;
      uint8_t unknown2;
      uint8_t count1;
      uint8_t count2;
    } __attribute__((__packed__));

    struct KeyMsg {
      uint8_t zero1;
      uint8_t modifier;
      uint8_t scancode1;
      uint8_t scancode2;
      uint8_t zero3;
    } __attribute__((__packed__));

    union {
      ClockMsg clock;
      KeyMsg key;
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
  libusb_device_handle* m_handle;
  uint16_t m_bcdDevice;
  bool m_no_init;
  bool m_debug;


  bool m_quit_thread;
  std::unique_ptr<uinpp::Device> m_uinput;
  std::unique_ptr<uinpp::GlibDevice> m_glib_uinput;
  std::array<uint16_t, 256> m_keymap;
  std::array<bool, 256> m_state;
  unsigned int m_led_state;
  libusb_transfer* m_read_transfer;

public:
  Chatpad(libusb_device_handle* handle, uint16_t bcdDevice,
          bool no_init, bool debug);
  ~Chatpad();

  void send_init();

  void set_led(unsigned int led, bool state);
  bool get_led(unsigned int led);

  void process(const ChatpadKeyMsg& msg);
  void init_uinput();

private:
  void send_command();
  void send_timeout(int msec);
  void send_ctrl(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
                 uint8_t* data_in = NULL, uint16_t length = 0,
                 libusb_transfer_cb_fn callback = NULL, void* userdata = NULL);

  void usb_submit_read(int endpoint, int len);

private:
  bool on_timeout();
  static gboolean on_timeout_wrap(gpointer data) {
    return static_cast<Chatpad*>(data)->on_timeout();
  }

  void on_control(libusb_transfer* transfer);
  static void on_control_wrap(libusb_transfer* transfer)
  {
    static_cast<Chatpad*>(transfer->user_data)->on_control(transfer);
  }

  void on_read_data(libusb_transfer* transfer);
  static void on_read_data_wrap(libusb_transfer* transfer)
  {
    static_cast<Chatpad*>(transfer->user_data)->on_read_data(transfer);
  }

private:
  Chatpad(const Chatpad&);
  Chatpad& operator=(const Chatpad&);
};

} // namespace xboxdrv

#endif

/* EOF */
