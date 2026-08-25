/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "wireless_chatpad.hpp"

#include <algorithm>
#include <linux/input.h>

#include <logmich/log.hpp>


namespace xboxdrv {

namespace {

// OUT payload length used by the wireless receiver for chatpad/control cmds.
constexpr int kOutLen = 12;

} // namespace

WirelessChatpad::WirelessChatpad(WriteFn write, bool no_init, bool debug) :
  m_write(std::move(write)),
  m_debug(debug),
  m_no_init(no_init),
  m_present(false),
  m_uinput(),
  m_glib_uinput(),
  m_keymap(),
  m_state(),
  m_sticky_shift(false),
  m_sticky_green(false),
  m_sticky_orange(false),
  m_sticky_people(false),
  m_caps_lock(false),
  m_eff_shift(false),
  m_eff_green(false),
  m_eff_orange(false),
  m_eff_people(false),
  m_led_state(0),
  m_timeout_source(0),
  m_keepalive_toggle(false),
  m_need_init(true)
{
  std::fill(m_keymap.begin(), m_keymap.end(), 0);
  std::fill(m_state.begin(), m_state.end(), false);

  // Same scancode map as wired Chatpad (US labels).
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
  m_keymap[CHATPAD_MOD_SHIFT] = KEY_LEFTSHIFT;
  m_keymap[CHATPAD_MOD_GREEN] = KEY_LEFTALT;
  m_keymap[CHATPAD_MOD_ORANGE] = KEY_LEFTCTRL;
  m_keymap[CHATPAD_MOD_PEOPLE] = KEY_LEFTMETA;

  init_uinput();
}

WirelessChatpad::~WirelessChatpad()
{
  stop_timeout();
  m_glib_uinput.reset();
  m_uinput.reset();
}

void
WirelessChatpad::init_uinput()
{
  struct input_id usbid = {};
  usbid.bustype = BUS_USB;
  usbid.vendor  = 0x045e;
  usbid.product = 0x0291; // wireless receiver family
  usbid.version = 0x0100;
  m_uinput = std::make_unique<uinpp::Device>(
    uinpp::DeviceType::KEYBOARD, "Xbox360 Wireless Chatpad", usbid);

  for (int i = 0; i < 256; ++i)
  {
    if (m_keymap[i])
    {
      m_uinput->add_key(m_keymap[i]);
    }
  }
  m_uinput->finish();
  m_glib_uinput = std::make_unique<uinpp::GlibDevice>(*m_uinput);
}

void
WirelessChatpad::send_cmd(uint8_t cmd)
{
  uint8_t pkt[kOutLen] = {
    0x00, 0x00, 0x0C, cmd,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  if (m_debug)
  {
    log_info("wireless-chatpad OUT cmd={:02x}", cmd);
  }
  m_write(pkt, kOutLen);
}

void
WirelessChatpad::send_timeout(int msec)
{
  stop_timeout();
  m_timeout_source = g_timeout_add(msec, &WirelessChatpad::on_timeout_wrap, this);
}

void
WirelessChatpad::stop_timeout()
{
  if (m_timeout_source)
  {
    g_source_remove(m_timeout_source);
    m_timeout_source = 0;
  }
}

gboolean
WirelessChatpad::on_timeout_wrap(gpointer data)
{
  return static_cast<WirelessChatpad*>(data)->on_timeout() ? TRUE : FALSE;
}

bool
WirelessChatpad::on_timeout()
{
  if (!m_present)
  {
    return false;
  }

  if (m_need_init && !m_no_init)
  {
    send_cmd(0x1B); // Chatpad enable / init
    m_need_init = false;
    // Keep-alives start after init
    send_timeout(1000);
    return true;
  }

  // Alternate keep-alive codes; chatpad stops streaming without them.
  send_cmd(m_keepalive_toggle ? 0x1F : 0x1E);
  m_keepalive_toggle = !m_keepalive_toggle;
  send_timeout(1000);
  return true;
}

void
WirelessChatpad::set_controller_present(bool present)
{
  if (m_present == present)
  {
    return;
  }
  m_present = present;
  if (present)
  {
    log_info("wireless-chatpad: controller present, starting init/keep-alive");
    m_need_init = !m_no_init;
    if (m_no_init)
    {
      // Still need keep-alives for streaming
      send_timeout(1000);
    }
    else
    {
      // Small delay then init
      send_timeout(100);
    }
  }
  else
  {
    log_info("wireless-chatpad: controller gone, stopping keep-alive");
    stop_timeout();
    m_need_init = true;
  }
}

bool
WirelessChatpad::handle_input(uint8_t const* data, int len)
{
  // Chatpad subtype: byte1 == 0x02 (issue #209 / Kytech).
  // Need payload through offset 0x18+3.
  if (len < 28 || data[1] != 0x02)
  {
    return false;
  }

  uint8_t const* payload = data + 0x18; // offset 24

  if (payload[0] == 0xF0)
  {
    // Status / handshake
    if (payload[1] == 0x03)
    {
      if (m_debug)
      {
        log_info("wireless-chatpad: handshake request, will re-init");
      }
      m_need_init = !m_no_init;
    }
    else if (payload[1] == 0x04 && m_debug)
    {
      log_info("wireless-chatpad: LED status {:02x}", payload[2]);
    }
    return true;
  }

  if (payload[0] == 0x00)
  {
    KeyMsg msg;
    msg.modifier = payload[1];
    msg.scancode1 = payload[2];
    msg.scancode2 = payload[3];
    process_key(msg);
    return true;
  }

  if (m_debug)
  {
    log_info("wireless-chatpad: unknown payload {:02x} {:02x} {:02x} {:02x}",
             payload[0], payload[1], payload[2], payload[3]);
  }
  return true;
}

void
WirelessChatpad::process_key(KeyMsg const& msg)
{
  if (m_debug)
  {
    log_info("wireless chatpad report: mod={:02x} key1={:02x} key2={:02x}",
             msg.modifier, msg.scancode1, msg.scancode2);
  }

  std::array<bool, 256> old_phys = m_state;
  std::fill(m_state.begin(), m_state.end(), false);

  // Modifier nibble (Cliffle / MS protocol): bit0=Shift, bit1=Green,
  // bit2=Orange, bit3=People. Store as pure bools (not the bit value).
  m_state[CHATPAD_MOD_SHIFT]  = (msg.modifier & CHATPAD_MOD_SHIFT)  != 0;
  m_state[CHATPAD_MOD_GREEN]  = (msg.modifier & CHATPAD_MOD_GREEN)  != 0;
  m_state[CHATPAD_MOD_ORANGE] = (msg.modifier & CHATPAD_MOD_ORANGE) != 0;
  m_state[CHATPAD_MOD_PEOPLE] = (msg.modifier & CHATPAD_MOD_PEOPLE) != 0;

  if (msg.scancode1) m_state[msg.scancode1] = true;
  if (msg.scancode2) m_state[msg.scancode2] = true;

  const bool rise_shift  = m_state[CHATPAD_MOD_SHIFT]  && !old_phys[CHATPAD_MOD_SHIFT];
  const bool rise_green  = m_state[CHATPAD_MOD_GREEN]  && !old_phys[CHATPAD_MOD_GREEN];
  const bool rise_orange = m_state[CHATPAD_MOD_ORANGE] && !old_phys[CHATPAD_MOD_ORANGE];
  const bool rise_people = m_state[CHATPAD_MOD_PEOPLE] && !old_phys[CHATPAD_MOD_PEOPLE];

  // Orange+Shift (either order) toggles CAPS lock, matching console policy.
  // The combo is consumed for CAPS and does not also arm one-shot stickies.
  bool caps_combo = false;
  if ((rise_shift && (m_state[CHATPAD_MOD_ORANGE] || m_sticky_orange)) ||
      (rise_orange && (m_state[CHATPAD_MOD_SHIFT] || m_sticky_shift || m_caps_lock)))
  {
    m_caps_lock = !m_caps_lock;
    m_sticky_shift  = false;
    m_sticky_orange = false;
    caps_combo = true;
    if (m_debug)
    {
      log_info("wireless chatpad CAPS lock {}", m_caps_lock ? "on" : "off");
    }
  }

  // One-shot sticky arm on tap (Xbox: mode armed until next text key).
  // Physical hold already keeps the modifier active via m_state.
  if (!caps_combo)
  {
    if (rise_green)  m_sticky_green  = true;
    if (rise_orange) m_sticky_orange = true;
    if (rise_people) m_sticky_people = true;
    if (rise_shift)  m_sticky_shift  = true;
  }

  // Any non-modifier key press consumes one-shot stickies after this report
  // has emitted the key with mods still active (console one-shot behaviour).
  bool text_key_pressed = false;
  for (size_t i = 0; i < m_state.size(); ++i)
  {
    const bool is_mod = (i == CHATPAD_MOD_SHIFT || i == CHATPAD_MOD_GREEN ||
                         i == CHATPAD_MOD_ORANGE || i == CHATPAD_MOD_PEOPLE);
    if (is_mod) continue;
    if (m_state[i] && !old_phys[i])
    {
      text_key_pressed = true;
      break;
    }
  }

  // Effective modifier down state: physical hold OR one-shot sticky OR CAPS.
  const bool eff_shift  = m_state[CHATPAD_MOD_SHIFT]  || m_sticky_shift  || m_caps_lock;
  const bool eff_green  = m_state[CHATPAD_MOD_GREEN]  || m_sticky_green;
  const bool eff_orange = m_state[CHATPAD_MOD_ORANGE] || m_sticky_orange;
  const bool eff_people = m_state[CHATPAD_MOD_PEOPLE] || m_sticky_people;

  // LEDs show software mode (armed / CAPS), not merely finger-down.
  set_led(CHATPAD_LED_SHIFT,  eff_shift);
  set_led(CHATPAD_LED_GREEN,  eff_green);
  set_led(CHATPAD_LED_ORANGE, eff_orange);
  set_led(CHATPAD_LED_PEOPLE, eff_people);

  auto emit = [this](size_t i, bool down) {
    if (m_keymap[i] == 0)
    {
      if (m_debug && down)
      {
        log_info("wireless chatpad unmapped scancode {:02x}", static_cast<unsigned>(i));
      }
      return;
    }
    m_uinput->send(EV_KEY, m_keymap[i], down ? 1 : 0);
  };

  // Previous effective state from last report (physical + sticky + caps).
  // We recompute old effective from old_phys and the sticky flags *before*
  // this report's arm/clear side effects where needed.
  // For emit edges, track what we last sent via dedicated previous-eff vars
  // stored in members updated at end of process.
  const bool old_eff_shift  = m_eff_shift;
  const bool old_eff_green  = m_eff_green;
  const bool old_eff_orange = m_eff_orange;
  const bool old_eff_people = m_eff_people;

  // Press order: modifiers first, then keys (so Shift+A capitalises).
  if (eff_shift  && !old_eff_shift)  emit(CHATPAD_MOD_SHIFT,  true);
  if (eff_green  && !old_eff_green)  emit(CHATPAD_MOD_GREEN,  true);
  if (eff_orange && !old_eff_orange) emit(CHATPAD_MOD_ORANGE, true);
  if (eff_people && !old_eff_people) emit(CHATPAD_MOD_PEOPLE, true);

  for (size_t i = 0; i < m_state.size(); ++i)
  {
    const bool is_mod = (i == CHATPAD_MOD_SHIFT || i == CHATPAD_MOD_GREEN ||
                         i == CHATPAD_MOD_ORANGE || i == CHATPAD_MOD_PEOPLE);
    if (is_mod) continue;
    if (m_state[i] && !old_phys[i])
      emit(i, true);
    else if (!m_state[i] && old_phys[i])
      emit(i, false);
  }

  // Consume one-shot stickies after the text key was emitted with mods down.
  if (text_key_pressed)
  {
    m_sticky_shift  = false;
    m_sticky_green  = false;
    m_sticky_orange = false;
    m_sticky_people = false;
  }

  // Recompute effective after sticky clear (physical + CAPS remain).
  const bool post_shift  = m_state[CHATPAD_MOD_SHIFT]  || m_sticky_shift  || m_caps_lock;
  const bool post_green  = m_state[CHATPAD_MOD_GREEN]  || m_sticky_green;
  const bool post_orange = m_state[CHATPAD_MOD_ORANGE] || m_sticky_orange;
  const bool post_people = m_state[CHATPAD_MOD_PEOPLE] || m_sticky_people;

  // Release order: keys first (already done), then modifiers.
  if (!post_shift  && old_eff_shift)  emit(CHATPAD_MOD_SHIFT,  false);
  if (!post_green  && old_eff_green)  emit(CHATPAD_MOD_GREEN,  false);
  if (!post_orange && old_eff_orange) emit(CHATPAD_MOD_ORANGE, false);
  if (!post_people && old_eff_people) emit(CHATPAD_MOD_PEOPLE, false);

  // If stickies were cleared, refresh LEDs to post-clear effective state.
  if (text_key_pressed)
  {
    set_led(CHATPAD_LED_SHIFT,  post_shift);
    set_led(CHATPAD_LED_GREEN,  post_green);
    set_led(CHATPAD_LED_ORANGE, post_orange);
    set_led(CHATPAD_LED_PEOPLE, post_people);
  }

  m_eff_shift  = post_shift;
  m_eff_green  = post_green;
  m_eff_orange = post_orange;
  m_eff_people = post_people;

  m_uinput->sync();
}

void
WirelessChatpad::set_led(unsigned int led, bool state)
{
  // Wireless LED codes from Kytech / issue #209 (ON / OFF pairs).
  uint8_t on = 0;
  uint8_t off = 0;
  if (led == CHATPAD_LED_SHIFT)
  {
    on = 0x08; off = 0x00; // Capslock / shift
  }
  else if (led == CHATPAD_LED_GREEN)
  {
    on = 0x09; off = 0x01;
  }
  else if (led == CHATPAD_LED_ORANGE)
  {
    on = 0x0A; off = 0x02;
  }
  else if (led == CHATPAD_LED_PEOPLE)
  {
    on = 0x0B; off = 0x03; // Messenger
  }
  else
  {
    return;
  }

  if (state)
  {
    m_led_state |= led;
    send_cmd(on);
  }
  else
  {
    m_led_state &= ~led;
    send_cmd(off);
  }
}

bool
WirelessChatpad::get_led(unsigned int led) const
{
  return (m_led_state & led) != 0;
}

} // namespace xboxdrv

/* EOF */
