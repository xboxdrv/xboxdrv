/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#ifndef HEADER_XBOXDRV_WIRELESS_CHATPAD_HPP
#define HEADER_XBOXDRV_WIRELESS_CHATPAD_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <glib.h>
#include <memory>

#include <uinpp/fwd.hpp>
#include <uinpp/glib_uinput.hpp>

#include "chatpad.hpp"

namespace xboxdrv {

/**
 * Chatpad on an Xbox 360 *wireless* controller (PC wireless receiver).
 *
 * Wired chatpad (see Chatpad) uses USB interface 2 and vendor control
 * transfers. Wireless multiplexes chatpad traffic on the same interrupt
 * IN/OUT endpoint as the gamepad:
 *
 *   OUT 12 bytes: 00 00 0C <cmd> + padding
 *        cmd 0x1B = enable/init, 0x1E/0x1F = keep-alive (alternate ~1 Hz)
 *   IN  subtype 0x02 (byte 1): key/status payload at offset 0x18
 *
 * References: github.com/xboxdrv/xboxdrv/issues/209,
 * github.com/Kytech/xbox360wirelesschatpad
 */
class WirelessChatpad
{
public:
  using WriteFn = std::function<void(uint8_t* data, int len)>;

  struct KeyMsg {
    uint8_t modifier;
    uint8_t scancode1;
    uint8_t scancode2;
  };

private:
  WriteFn m_write;
  bool m_debug;
  bool m_no_init;
  bool m_present;

  std::unique_ptr<uinpp::Device> m_uinput;
  std::unique_ptr<uinpp::GlibDevice> m_glib_uinput;
  std::array<uint16_t, 256> m_keymap;
  std::array<bool, 256> m_state;
  // One-shot sticky (same policy as wired Chatpad); no CAPS special case.
  bool m_sticky_shift;
  bool m_sticky_green;
  bool m_sticky_orange;
  bool m_sticky_people;
  bool m_eff_shift;
  bool m_eff_green;
  bool m_eff_orange;
  bool m_eff_people;
  unsigned int m_led_state;

  guint m_timeout_source;
  bool m_keepalive_toggle;
  bool m_need_init;

public:
  WirelessChatpad(WriteFn write, bool no_init, bool debug);
  ~WirelessChatpad();

  void set_controller_present(bool present);

  /** Full wireless IN report. True if packet was chatpad traffic. */
  bool handle_input(uint8_t const* data, int len);

  void set_led(unsigned int led, bool state);
  bool get_led(unsigned int led) const;

private:
  void init_uinput();
  void send_cmd(uint8_t cmd);
  void send_timeout(int msec);
  void stop_timeout();
  bool on_timeout();
  static gboolean on_timeout_wrap(gpointer data);

  void process_key(KeyMsg const& msg);

  WirelessChatpad(WirelessChatpad const&) = delete;
  WirelessChatpad& operator=(WirelessChatpad const&) = delete;
};

} // namespace xboxdrv

#endif

/* EOF */
