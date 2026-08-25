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

#include <assert.h>
#include <linux/input.h>
#include <sys/time.h>

#include <uinpp/device.hpp>
#include <logmich/log.hpp>
#include <unsebu/usb_helper.hpp>

#include "raise_exception.hpp"

namespace xboxdrv {

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
  m_interface_claimed(false),
  m_keepalive_stalls(0),
  m_quit_thread(false),
  m_uinput(),
  m_glib_uinput(),
  m_keymap(),
  m_state(),
  m_led_state(0),
  m_read_transfer(nullptr),
  m_timeout_source(0)
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
  // Scancode 0x22 is physical "Y" position on US keycaps, "Z" on German QWERTZ
  // (see docs/chatpad-layout.md). Default map follows US labels.
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
  // Scancode 0x46 is physical "Z" on US, "Y" on German QWERTZ.
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

  // Chatpad lives on interface 2 (vendor-specific). The gamepad claims
  // interface 0; headset uses 1. Without claiming 2, control and interrupt
  // transfers fail or race with the kernel.
  {
    int err = unsebu::usb_claim_n_detach_interface(m_handle, 2, true);
    if (err != 0)
    {
      raise_exception(std::runtime_error,
                      "couldn't claim chatpad USB interface 2: " << libusb_strerror(err)
                      << " (try --detach-kernel-driver)");
    }
    m_interface_claimed = true;
  }

  init_uinput();

  if (no_init)
  {
    // Skip the heavy init sequence (safer when restarting xboxdrv without
    // unplugging the pad — full init can lock up some firmwares).
    m_init_state = kStateKeepAlive_1e;
    if (m_bcdDevice == 0x0110)
    {
      usb_submit_read(6, 32);
    }
    else if (m_bcdDevice == 0x0114)
    {
      usb_submit_read(4, 32);
    }
    log_info("chatpad: skipping init (--chatpad-no-init), listening for key reports");
  }

  // Start the init / keep-alive state machine. Full init starts interrupt
  // reads only after the 0x1b enable step (see on_control).
  send_command();
}

Chatpad::~Chatpad()
{
  if (m_timeout_source)
  {
    g_source_remove(m_timeout_source);
    m_timeout_source = 0;
  }

  if (m_read_transfer)
  {
    // Cancel and wait for the completion callback; do not free here.
    int ret = libusb_cancel_transfer(m_read_transfer);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND)
    {
      log_error("chatpad: libusb_cancel_transfer failed: {}", libusb_strerror(ret));
    }

    for (int i = 0; m_read_transfer && i < 100; ++i)
    {
      timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000;
      ret = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
      if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_INTERRUPTED)
      {
        log_error("chatpad: event drain failed: {}", libusb_strerror(ret));
        break;
      }
    }

    if (m_read_transfer)
    {
      log_error("chatpad: read transfer still pending after cancel; abandoning");
      m_read_transfer = nullptr;
    }
  }

  m_glib_uinput.reset();
  m_uinput.reset();

  if (m_interface_claimed)
  {
    int ret = libusb_release_interface(m_handle, 2);
    if (ret != LIBUSB_SUCCESS)
    {
      log_debug("chatpad: release interface 2 failed: {}", libusb_strerror(ret));
    }
    m_interface_claimed = false;
  }
}

void
Chatpad::init_uinput()
{
  struct input_id usbid;

  // Present as a real USB keyboard so compositors apply Shift/modifiers
  // to this device's key events (GENERIC is often ignored for text input).
  usbid.bustype = BUS_USB;
  usbid.vendor  = 0x045e; // Microsoft
  usbid.product = 0x028e; // same family as the wired pad
  usbid.version = m_bcdDevice;

  m_uinput = std::make_unique<uinpp::Device>(uinpp::DeviceType::KEYBOARD, "Xbox360 Chatpad", usbid);

  for(int i = 0; i < 256; ++i)
  {
    if (m_keymap[i])
    {
      m_uinput->add_key(m_keymap[i]);
    }
  }
  m_uinput->finish();

  // register glib callbacks
  m_glib_uinput = std::make_unique<uinpp::GlibDevice>(*m_uinput);
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
    libusb_free_transfer(m_read_transfer);
    m_read_transfer = nullptr;
    raise_exception(std::runtime_error, "chatpad libusb_submit_transfer(): " << libusb_strerror(ret));
  }
}

void
Chatpad::on_read_data(libusb_transfer* transfer)
{
  assert(transfer);
  assert(transfer == m_read_transfer);

  if (transfer->status == LIBUSB_TRANSFER_CANCELLED)
  {
    libusb_free_transfer(transfer);
    m_read_transfer = nullptr;
    return;
  }

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
  {
    log_error("chatpad usb transfer failed: {}", libusb_error_name(transfer->status));
    libusb_free_transfer(transfer);
    m_read_transfer = nullptr;
    return;
  }

  // Key reports are 5 bytes: 00 | mod | key1 | key2 | 00 (Cliffle/MS layout).
  if (transfer->actual_length >= 5 && transfer->buffer[0] == 0x00)
  {
    ChatpadKeyMsg msg{};
    msg.zero1     = transfer->buffer[0];
    msg.modifier  = transfer->buffer[1];
    msg.scancode1 = transfer->buffer[2];
    msg.scancode2 = transfer->buffer[3];
    msg.zero3     = transfer->buffer[4];
    process(msg);
  }
  else if (m_debug)
  {
    log_info("chatpad ignored transfer len={} status={}",
             transfer->actual_length, libusb_error_name(transfer->status));
  }

  int ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    log_error("chatpad failed to resubmit USB transfer: {}", libusb_strerror(ret));
    libusb_free_transfer(transfer);
    m_read_transfer = nullptr;
  }
}

void
Chatpad::send_timeout(int msec)
{
  if (m_timeout_source)
  {
    g_source_remove(m_timeout_source);
    m_timeout_source = 0;
  }
  m_timeout_source = g_timeout_add(msec, &Chatpad::on_timeout_wrap, this);
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
  State const state = m_init_state;
  bool const ok = (transfer->status == LIBUSB_TRANSFER_COMPLETED);

  if (!ok)
  {
    // STALL is common when the pad is already initialised or mid-reinit.
    // Clear the control pipe and continue; do not spam ERROR every second.
    if (transfer->status == LIBUSB_TRANSFER_STALL)
    {
      libusb_clear_halt(m_handle, 0);
      if (state == kStateKeepAlive_1e || state == kStateKeepAlive_1f)
      {
        ++m_keepalive_stalls;
        if (m_keepalive_stalls <= 3 || (m_keepalive_stalls % 30) == 0)
        {
          log_warn("chatpad keep-alive STALL (count={}); cleared halt — "
                   "try --chatpad-no-init if this persists after a restart",
                   m_keepalive_stalls);
        }
      }
      else
      {
        log_warn("chatpad init control STALL in state {}: {}",
                 static_cast<int>(state), libusb_error_name(transfer->status));
      }
    }
    else
    {
      log_error("chatpad control transfer status: {}", libusb_error_name(transfer->status));
    }
  }
  else if (state == kStateKeepAlive_1e || state == kStateKeepAlive_1f)
  {
    m_keepalive_stalls = 0;
  }

  switch(state)
  {
    case kStateInit1:
    case kStateInit2:
    case kStateInit3:
      // These probes often fail; continue regardless.
      m_init_state = static_cast<State>(state + 1);
      send_command();
      break;

    case kStateInit4:
    case kStateInit5:
    case kStateInit6:
    case kStateInit_1e:
    case kStateInit_1f:
      // Advance even on STALL so we still reach keep-alive / read start.
      m_init_state = static_cast<State>(state + 1);
      send_command();
      break;

    case kStateInit_1b:
      // After enable (0x1b), start interrupt reads then keep-alive loop.
      m_init_state = kStateKeepAlive_1e;
      if (!m_read_transfer)
      {
        if (m_bcdDevice == 0x0110)
        {
          usb_submit_read(6, 32);
        }
        else if (m_bcdDevice == 0x0114)
        {
          usb_submit_read(4, 32);
        }
        log_info("chatpad: init done, listening for key reports");
      }
      send_command();
      break;

    case kStateKeepAlive_1e:
    case kStateKeepAlive_1f:
      m_init_state = static_cast<State>(state + 1);
      if (m_init_state == kStateLoop)
      {
        m_init_state = kStateKeepAlive_1e;
      }
      send_command();
      break;

    default:
      assert(false && "unknown state");
      break;
  }
}

bool
Chatpad::on_timeout()
{
  m_timeout_source = 0;
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
  if (!m_handle || !m_interface_claimed)
  {
    log_debug("chatpad: send_ctrl skipped (no handle/interface)");
    return;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
  transfer->flags |= LIBUSB_TRANSFER_FREE_TRANSFER;

  uint8_t* data = static_cast<uint8_t*>(malloc(length + 8));
  libusb_fill_control_setup(data, request_type, request, value, index, length);
  if (length && data_in)
  {
    memcpy(data + 8, data_in, length);
  }
  libusb_fill_control_transfer(transfer, m_handle, data,
                               callback, userdata,
                               0);

  int ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    // Keep-alives and LED toggles must not throw during unplug/shutdown.
    log_error("chatpad control transfer failed: {}", libusb_strerror(ret));
    libusb_free_transfer(transfer);
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
Chatpad::process(ChatpadKeyMsg const& msg)
{
  if (m_debug)
  {
    log_info("chatpad report: mod={:02x} key1={:02x} key2={:02x}",
             msg.modifier, msg.scancode1, msg.scancode2);
  }

  std::array<bool, 256> old_state = m_state;
  std::fill(m_state.begin(), m_state.end(), false);

  // Modifier nibble (Cliffle / MS protocol): bit0=Shift, bit1=Green,
  // bit2=Orange, bit3=People. Store as pure bools (not the bit value).
  m_state[CHATPAD_MOD_SHIFT]  = (msg.modifier & CHATPAD_MOD_SHIFT)  != 0;
  m_state[CHATPAD_MOD_GREEN]  = (msg.modifier & CHATPAD_MOD_GREEN)  != 0;
  m_state[CHATPAD_MOD_ORANGE] = (msg.modifier & CHATPAD_MOD_ORANGE) != 0;
  m_state[CHATPAD_MOD_PEOPLE] = (msg.modifier & CHATPAD_MOD_PEOPLE) != 0;

  if (msg.scancode1) m_state[msg.scancode1] = true;
  if (msg.scancode2) m_state[msg.scancode2] = true;

  // Shift LED tracks the held modifier bit (not sticky).
  if (m_state[CHATPAD_MOD_SHIFT] != old_state[CHATPAD_MOD_SHIFT])
  {
    set_led(CHATPAD_LED_SHIFT, m_state[CHATPAD_MOD_SHIFT]);
  }

  auto emit = [this](size_t i, bool down) {
    if (m_keymap[i] == 0)
    {
      if (m_debug && down)
      {
        log_info("chatpad unmapped scancode {:02x}", static_cast<unsigned>(i));
      }
      return;
    }
    m_uinput->send(EV_KEY, m_keymap[i], down ? 1 : 0);
  };

  // Press order: modifiers first, then keys (so Shift+A capitalises).
  // Release order: keys first, then modifiers.
  for (size_t i = 0; i < m_state.size(); ++i)
  {
    if (m_state[i] && !old_state[i])
    {
      // LED policy (see docs/chatpad-layout.md):
      // - Shift is momentary; LED follows held state (updated below).
      // - Green / Orange / People: toggle on press for now (console-style
      //   lock feedback). Orange+Shift CAPS sticky is not implemented yet.
      if (i == CHATPAD_MOD_PEOPLE)
        set_led(CHATPAD_LED_PEOPLE, !get_led(CHATPAD_LED_PEOPLE));
      else if (i == CHATPAD_MOD_ORANGE)
        set_led(CHATPAD_LED_ORANGE, !get_led(CHATPAD_LED_ORANGE));
      else if (i == CHATPAD_MOD_GREEN)
        set_led(CHATPAD_LED_GREEN, !get_led(CHATPAD_LED_GREEN));

      if (i == CHATPAD_MOD_SHIFT || i == CHATPAD_MOD_GREEN ||
          i == CHATPAD_MOD_ORANGE || i == CHATPAD_MOD_PEOPLE)
      {
        emit(i, true);
      }
    }
  }

  for (size_t i = 0; i < m_state.size(); ++i)
  {
    bool is_mod = (i == CHATPAD_MOD_SHIFT || i == CHATPAD_MOD_GREEN ||
                   i == CHATPAD_MOD_ORANGE || i == CHATPAD_MOD_PEOPLE);
    if (is_mod) continue;
    if (m_state[i] && !old_state[i])
      emit(i, true);
    else if (!m_state[i] && old_state[i])
      emit(i, false);
  }

  for (size_t i = 0; i < m_state.size(); ++i)
  {
    bool is_mod = (i == CHATPAD_MOD_SHIFT || i == CHATPAD_MOD_GREEN ||
                   i == CHATPAD_MOD_ORANGE || i == CHATPAD_MOD_PEOPLE);
    if (!is_mod) continue;
    if (!m_state[i] && old_state[i])
      emit(i, false);
  }

  m_uinput->sync();
}

} // namespace xboxdrv

/* EOF */
