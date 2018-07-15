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

#include "chatpad.hpp"

#include "helper.hpp"
#include "linux_uinput.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"

struct USBControlMsg
{
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  unsigned char *data;
  uint16_t wLength;
};

/*
  Chatpad Interface:
  ==================
  bInterfaceNumber        2
  bInterfaceClass       255 Vendor Specific Class
  bInterfaceSubClass     93
  bInterfaceProtocol      2
*/
Chatpad::Chatpad(libusb_device_handle* handle, uint16_t bcdDevice,
                 bool no_init, bool debug) :
  m_init_state(kStateInit1),
  m_handle(handle),
  m_bcdDevice(bcdDevice),
  m_no_init(no_init),
  m_debug(debug),
  m_quit_thread(false),
  m_uinput(),
  m_keymap(),
  m_state(),
  m_led_state(0),
  m_read_transfer(0)
{
  if (m_bcdDevice != 0x0110 && m_bcdDevice != 0x0114)
  {
    throw std::runtime_error("unknown bcdDevice version number, please report this issue "
                             "to <grumbel@gmail.com> and include the output of 'lsusb -v'");
  }

  std::fill(m_keymap.begin(), m_keymap.end(), 0);
  std::fill(m_state.begin(), m_state.end(), 0);

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

  if (no_init)
  {
    m_init_state = kStateKeepAlive_1e;
  }

  send_command();

  if (m_bcdDevice == 0x0110)
  {
    usb_submit_read(6, 32);
  }
  else if (m_bcdDevice == 0x0114)
  {
    usb_submit_read(4, 32);
  }
}

Chatpad::~Chatpad()
{
  if (m_read_transfer)
  {
    libusb_cancel_transfer(m_read_transfer);
    libusb_free_transfer(m_read_transfer);
  }
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

void
Chatpad::usb_submit_read(int endpoint, int len)
{
  assert(!m_read_transfer);

  m_read_transfer = libusb_alloc_transfer(0);

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  m_read_transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  libusb_fill_interrupt_transfer(m_read_transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_IN),
                                 data, len,
                                 &Chatpad::on_read_data_wrap, this,
                                 0); // timeout
  int ret;
  ret = libusb_submit_transfer(m_read_transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
}

void
Chatpad::on_read_data(libusb_transfer* transfer)
{
  assert(transfer);

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
  {
    log_error("usb transfer failed: " << usb_transfer_strerror(transfer->status));
  }
  else
  {
    //log_tmp("chatpad data: " << usb_transfer_strerror(transfer->status) << " "
    //        << raw2str(transfer->buffer, transfer->actual_length));

    if (transfer->actual_length == 5 && transfer->buffer[0] == 0x00)
    {
      struct ChatpadKeyMsg msg;
      memcpy(&msg, transfer->buffer, transfer->actual_length);
      process(msg);
    }

    int ret;
    ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS)
    {
      log_error("failed to resubmit USB transfer: " << usb_strerror(ret));
      libusb_free_transfer(transfer);
    }
  }
}

void
Chatpad::send_timeout(int msec)
{
  // FIMXE: must keep track of sources and destroy them in ~Chatpad()
  //assert(m_timeout_id == -1);
  //m_timeout_id =
  g_timeout_add(1000, &Chatpad::on_timeout_wrap, this);
}

void
Chatpad::send_command()
{
  //log_tmp("send_command: " << m_init_state);

  // default init code for m_bcdDevice == 0x0110
  uint8_t code[2] = { 0x01, 0x02 };

  if (m_bcdDevice == 0x0114)
  {
    code[0] = 0x09;
    code[1] = 0x00;
  }

  switch(m_init_state)
  {
    case kStateInit1:
      send_ctrl(0x40, 0xa9, 0xa30c, 0x4423, NULL, 0,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit2:
      send_ctrl(0x40, 0xa9, 0x2344, 0x7f03, NULL, 0,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit3:
      send_ctrl(0x40, 0xa9, 0x5839, 0x6832, NULL, 0,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit4:
      send_ctrl(0xc0, 0xa1, 0x0000, 0xe416, code, 2,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit5:
      send_ctrl(0x40, 0xa1, 0x0000, 0xe416, code, 2,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit6:
      send_ctrl(0xc0, 0xa1, 0x0000, 0xe416, code, 2,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateInit_1e:
      send_timeout(1000);
      break;

    case kStateInit_1f:
      send_timeout(1000);
      break;

    case kStateInit_1b:
      send_ctrl(0x41, 0x0, 0x1b, 0x02, NULL, 0,
                &Chatpad::on_control_wrap, this);
      break;

    case kStateKeepAlive_1e:
      send_timeout(1000);
      break;

    case kStateKeepAlive_1f:
      send_timeout(1000);
      break;

    default:
      assert(false && "unknown state");
      break;
  }
}

void
Chatpad::on_control(libusb_transfer* transfer)
{
  //log_tmp(m_init_state << " " << usb_transfer_strerror(transfer->status) << " len: " << transfer->actual_length);
  if (transfer->actual_length != 0)
  {
    //log_tmp(" PAYLOAD: " << raw2str(transfer->buffer, transfer->actual_length));
  }

  switch(m_init_state)
  {
    case kStateInit1:
    case kStateInit2:
    case kStateInit3:
      // fail ok
      m_init_state = static_cast<State>(m_init_state + 1);
      send_command();
      break;

    case kStateInit4:
    case kStateInit5:
    case kStateInit6:
    case kStateInit_1e:
    case kStateInit_1f:
    case kStateInit_1b:
    case kStateKeepAlive_1e:
    case kStateKeepAlive_1f:
      if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
      {
        log_error("stuff went wrong");
      }
      else
      {
        m_init_state = static_cast<State>(m_init_state + 1);
        if (m_init_state == kStateLoop)
        {
          m_init_state = kStateKeepAlive_1e;
        }
        send_command();
      }
      break;

    default:
      assert(false && "unknown state");
      break;
  }
}

bool
Chatpad::on_timeout()
{
  //m_timeout_id = -1;
  switch(m_init_state)
  {
    case kStateInit_1e:
    case kStateKeepAlive_1e:
      send_ctrl(0x41, 0x0, 0x1f, 0x02, NULL, 0,
                &Chatpad::on_control_wrap, this);
      return false;

    case kStateInit_1f:
    case kStateKeepAlive_1f:
      send_ctrl(0x41, 0x0, 0x1e, 0x02, NULL, 0,
                &Chatpad::on_control_wrap, this);
      return false;

    default:
      assert(false && "invalid state");
      return false;
  }
}

void
Chatpad::send_ctrl(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
                   uint8_t* data_in, uint16_t length,
                   libusb_transfer_cb_fn callback, void* userdata)
{
  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  transfer->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;

  // create and fill control buffer
  uint8_t* data = static_cast<uint8_t*>(malloc(length + 8));
  libusb_fill_control_setup(data, request_type, request, value, index, length);
  memcpy(data + 8, data_in, length);
  libusb_fill_control_transfer(transfer, m_handle, data,
                               callback, userdata,
                               0);

  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
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
Chatpad::process(const ChatpadKeyMsg& msg)
{
  // save old state
  boost::array<bool, 256> old_state = m_state;

  // generate new state
  std::fill(m_state.begin(), m_state.end(), 0);

  m_state[CHATPAD_MOD_PEOPLE] = msg.modifier & CHATPAD_MOD_PEOPLE;
  m_state[CHATPAD_MOD_ORANGE] = msg.modifier & CHATPAD_MOD_ORANGE;
  m_state[CHATPAD_MOD_GREEN]  = msg.modifier & CHATPAD_MOD_GREEN;
  m_state[CHATPAD_MOD_SHIFT]  = msg.modifier & CHATPAD_MOD_SHIFT;

  if (msg.scancode1) m_state[msg.scancode1] = true;
  if (msg.scancode2) m_state[msg.scancode2] = true;

  // check for changes
  for(size_t i = 0; i < m_state.size(); ++i)
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

/* EOF */
