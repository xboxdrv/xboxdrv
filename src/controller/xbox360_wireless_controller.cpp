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

#include "controller/xbox360_wireless_controller.hpp"

#include <chrono>
#include <sstream>
#include <format>
#include <iostream>

#include <unsebu/usb_helper.hpp>

#include "controller_message.hpp"
#include "util/string.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"
#include "headset.hpp"
#include "wireless_chatpad.hpp"

namespace xboxdrv {

Xbox360WirelessController::Xbox360WirelessController(libusb_device* dev, int controller_id,
                                                     bool chatpad, bool chatpad_no_init, bool chatpad_debug,
                                                     bool headset,
                                                     bool headset_debug,
                                                     std::string const& headset_dump,
                                                     std::string const& headset_play,
                                                     std::string const& headset_pcm,
                                                     std::string const& headset_wav,
                                                     std::string const& headset_play_wav,
                                                     bool headset_play_left_pack,
                                                     bool headset_pulse,
                                                     bool headset_pipewire,
                                                     float headset_mic_gain,
                                                     bool try_detach,
                                                     bool auto_poweroff,
                                                     int guide_poweroff_timeout_sec,
                                                     bool quiet) :
  USBController(dev),
  m_endpoint(),
  m_interface(),
  m_battery_status(),
  m_serial(),
  m_chatpad(),
  m_headset(),
  m_auto_poweroff(auto_poweroff),
  m_guide_poweroff_timeout_sec(guide_poweroff_timeout_sec),
  m_quiet(quiet),
  m_pad_present(false),
  m_got_pad_report(false),
  m_guide_down_ts(),
  m_guide_held(false),
  m_guide_timeout_source(0),
  m_zombie_check_source(0),
  xbox(m_message_descriptor)
{
  // FIXME: A little bit of a hack
  m_is_active = false;

  assert(controller_id >= 0 && controller_id < 4);

  // FIXME: Is hardcoding those ok?
  m_endpoint  = controller_id*2 + 1;
  m_interface = controller_id*2;

  usb_claim_interface(m_interface, try_detach);
  usb_submit_read(m_endpoint, 32);

  // Same one-shot as kernel xpad360w_start_input. Does not recover a pad that
  // was powered on before the host and stopped streaming input (see man page).
  inquire_presence();

  // One advisory if we never see a real pad report (zombie session).
  m_zombie_check_source = g_timeout_add_seconds(
    5, &Xbox360WirelessController::on_zombie_check_wrap, this);

  if (chatpad)
  {
    m_chatpad = std::make_unique<WirelessChatpad>(
      [this](uint8_t* data, int len) {
        usb_write(m_endpoint, data, len);
      },
      chatpad_no_init,
      chatpad_debug);
  }

  // Headset lives on the odd interface next to this controller slot
  // (PROTOCOL: IF 1/EP 2 for slot 0, IF 3/EP 4 for slot 1, …).
  // Framing is assumed to match wired 32-byte G.726 interrupt transfers;
  // verify with hardware — wireless may need further protocol work.
  if (headset)
  {
    const int hs_if = controller_id * 2 + 1;
    const int hs_ep = controller_id * 2 + 2;
    log_info("[headset] wireless slot {}: interface {} endpoint {}",
             controller_id, hs_if, hs_ep);
    m_headset = std::make_unique<Headset>(
      m_handle, headset_debug, headset_mic_gain,
      hs_if, hs_ep, hs_ep, try_detach);

    if (!headset_play.empty())
    {
      m_headset->play_file(headset_play);
    }
    if (!headset_dump.empty())
    {
      m_headset->record_file(headset_dump);
    }
    if (!headset_pcm.empty())
    {
      m_headset->record_pcm(headset_pcm);
    }
    if (!headset_wav.empty())
    {
      m_headset->record_wav(headset_wav);
    }
    if (!headset_play_wav.empty())
    {
      m_headset->play_wav(headset_play_wav, headset_play_left_pack);
    }
    if (headset_pulse)
    {
      m_headset->enable_pulse_audio();
    }
    if (headset_pipewire)
    {
      m_headset->enable_pipewire_audio();
    }
  }
}

Xbox360WirelessController::~Xbox360WirelessController()
{
  stop_zombie_check();
  stop_guide_timeout();

  // Mirror kernel xpad auto_poweroff on suspend: if the pad is still
  // linked when we tear down the slot, power it down so it does not
  // flash and drain batteries while searching for a missing receiver.
  if (m_auto_poweroff && m_pad_present && !is_disconnected())
  {
    power_off();
  }
}


void
Xbox360WirelessController::inquire_presence()
{
  // Same 12-byte packet as kernel xpad_inquiry_pad_presence().
  uint8_t cmd[] = {
    0x08, 0x00, 0x0f, 0xc0,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  log_debug("wireless presence inquiry");
  usb_write(m_endpoint, cmd, sizeof(cmd));
}

void
Xbox360WirelessController::stop_zombie_check()
{
  if (m_zombie_check_source)
  {
    g_source_remove(m_zombie_check_source);
    m_zombie_check_source = 0;
  }
}

gboolean
Xbox360WirelessController::on_zombie_check_wrap(gpointer data)
{
  return static_cast<Xbox360WirelessController*>(data)->on_zombie_check()
    ? TRUE : FALSE;
}

bool
Xbox360WirelessController::on_zombie_check()
{
  m_zombie_check_source = 0;
  if (m_got_pad_report || is_disconnected() || m_shutting_down)
  {
    return false;
  }
  // LED may still work; neither xboxdrv nor xpad can recover this session.
  char const* msg =
    "No input from wireless controller yet. Power the controller on, or "
    "power-cycle it (remove batteries / battery pack briefly), then try again.";
  log_info("{}", msg);
  if (!m_quiet)
  {
    std::cout << msg << std::endl;
  }
  return false;
}

void
Xbox360WirelessController::power_off()
{
  // Same 12-byte packet as kernel xpad360w_poweroff_controller().
  uint8_t cmd[] = {
    0x00, 0x00, 0x08, 0xc0,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };
  stop_guide_timeout();
  log_info("wireless power-off");
  if (!m_quiet)
  {
    std::cout << "Wireless controller powered off" << std::endl;
  }
  usb_write(m_endpoint, cmd, sizeof(cmd));
  m_pad_present = false;
  m_guide_held = false;
}

void
Xbox360WirelessController::stop_guide_timeout()
{
  if (m_guide_timeout_source)
  {
    g_source_remove(m_guide_timeout_source);
    m_guide_timeout_source = 0;
  }
}

gboolean
Xbox360WirelessController::on_guide_timeout_wrap(gpointer data)
{
  return static_cast<Xbox360WirelessController*>(data)->on_guide_timeout()
    ? TRUE : FALSE;
}

bool
Xbox360WirelessController::on_guide_timeout()
{
  // One-shot: clear id first (same pattern as WirelessChatpad keep-alives).
  m_guide_timeout_source = 0;
  if (!m_guide_held || !m_pad_present)
  {
    return false;
  }
  log_info("guide held {}s — powering off wireless controller",
           m_guide_poweroff_timeout_sec);
  power_off();
  set_active(false);
  if (m_chatpad)
  {
    m_chatpad->set_controller_present(false);
  }
  return false;
}

void
Xbox360WirelessController::maybe_guide_poweroff(bool guide_down)
{
  if (m_guide_poweroff_timeout_sec <= 0)
  {
    stop_guide_timeout();
    m_guide_held = false;
    return;
  }

  if (guide_down)
  {
    if (!m_guide_held)
    {
      // Arm a real timer. Relying only on subsequent input reports is
      // unreliable: while the Guide button is held the pad may stop
      // streaming pad data, so the old "check elapsed on each report"
      // path never fired.
      m_guide_held = true;
      m_guide_down_ts = std::chrono::steady_clock::now();
      stop_guide_timeout();
      m_guide_timeout_source = g_timeout_add_seconds(
        static_cast<guint>(m_guide_poweroff_timeout_sec),
        &Xbox360WirelessController::on_guide_timeout_wrap,
        this);
    }
  }
  else
  {
    stop_guide_timeout();
    m_guide_held = false;
  }
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
Xbox360WirelessController::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (m_chatpad && m_chatpad->handle_input(data, len))
  {
    return false; // chatpad consumed; no gamepad message
  }

  if (len <= 0)
  {
    return false;
  }

  auto mark_present = [this]() {
    // Any sign of a live pad: attach to a daemon slot (if still inactive)
    // and start chatpad keep-alives. Battery-cycle was often needed only
    // because we required a fresh 0x08/0x80 and exact len==29 reports.
    m_pad_present = true;
    set_active(true);
    if (m_chatpad)
    {
      m_chatpad->set_controller_present(true);
    }
  };

  // Connection status (xpad: data[0] & 0x08). Short 2-byte form is common;
  // accept the presence nibble on longer frames too.
  if (len >= 2 && (data[0] & 0x08) != 0)
  {
    uint8_t st = data[1];
    if ((st & 0x80) == 0 && (st & 0x40) == 0)
    {
      // Presence inquiry can elicit this status repeatedly. Only react on a
      // real present→absent transition; never submit an empty ControllerMessage
      // (that showed up as all-zero "msg:" lines every poll without any pad
      // activity).
      if (m_pad_present)
      {
        log_info("connection status: nothing");
        m_pad_present = false;
        m_got_pad_report = false;
        m_guide_held = false;
        stop_guide_timeout();
        set_active(false);
        if (m_chatpad)
        {
          m_chatpad->set_controller_present(false);
        }
      }
      return false;
    }

    if (st & 0x80)
    {
      if (st & 0x40)
        log_info("connection status: controller and headset connected");
      else
        log_info("connection status: controller connected");
      // LED is also set when the daemon assigns a slot; nudge here so the
      // pad is awake even before the idle activation callback runs.
      set_led_real(get_led());
      mark_present();
      // Fall through: a longer frame may also carry pad data.
      if (len <= 2)
      {
        return false;
      }
    }
    else if (st & 0x40)
    {
      log_info("connection status: headset connected");
      if (len <= 2)
      {
        return false;
      }
    }
  }

  // Gamepad / status payloads (xpad: data[1] == 0x01 for pad data).
  // Do not require exact len==29: receivers report 20–32 byte actual lengths.
  if (len >= 18)
  {
    // Announce / presence meta (serial + battery)
    if (data[0] == 0x00 && data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
    {
      mark_present();
      if (len >= 18)
      {
        m_serial = std::format("{:2x}:{:2x}:{:2x}:{:2x}:{:2x}:{:2x}:{:2x}",
                               int(data[7]),
                               int(data[8]),
                               int(data[9]),
                               int(data[10]),
                               int(data[11]),
                               int(data[12]),
                               int(data[13]));
        m_battery_status = data[17];
        log_info("controller serial: {}", m_serial);
        log_info("battery status: {} (0=empty .. 3=full typical)", m_battery_status);
      }
      return false;
    }

    // Input report: match xpad (byte1 == 0x01). Payload starts at offset 4.
    // Previously required data[4]==0x00 && data[5]==0x13 which dropped some
    // firmware/receiver variants → LED on (slot active) but no events.
    if (data[1] == 0x01)
    {
      mark_present();
      m_got_pad_report = true;
      stop_zombie_check();

      uint8_t const* ptr = data + 4;

      msg_out->set_key(xbox.dpad_up,    unpack::bit(ptr+2, 0));
      msg_out->set_key(xbox.dpad_down,  unpack::bit(ptr+2, 1));
      msg_out->set_key(xbox.dpad_left,  unpack::bit(ptr+2, 2));
      msg_out->set_key(xbox.dpad_right, unpack::bit(ptr+2, 3));

      msg_out->set_key(xbox.btn_start,   unpack::bit(ptr+2, 4));
      msg_out->set_key(xbox.btn_back,    unpack::bit(ptr+2, 5));
      msg_out->set_key(xbox.btn_thumb_l, unpack::bit(ptr+2, 6));
      msg_out->set_key(xbox.btn_thumb_r, unpack::bit(ptr+2, 7));

      msg_out->set_key(xbox.btn_lb, unpack::bit(ptr+3, 0));
      msg_out->set_key(xbox.btn_rb, unpack::bit(ptr+3, 1));
      bool const guide_down = unpack::bit(ptr+3, 2);
      msg_out->set_key(xbox.btn_guide, guide_down);
      maybe_guide_poweroff(guide_down);

      msg_out->set_key(xbox.btn_a, unpack::bit(ptr+3, 4));
      msg_out->set_key(xbox.btn_b, unpack::bit(ptr+3, 5));
      msg_out->set_key(xbox.btn_x, unpack::bit(ptr+3, 6));
      msg_out->set_key(xbox.btn_y, unpack::bit(ptr+3, 7));

      msg_out->set_abs(xbox.abs_lt, ptr[4], 0, 255);
      msg_out->set_abs(xbox.abs_rt, ptr[5], 0, 255);

      msg_out->set_abs(xbox.abs_x1, unpack::int16le(ptr+6), -32768, 32767);
      msg_out->set_abs(xbox.abs_y1, unpack::s16_invert(unpack::int16le(ptr+8)), -32768, 32767);

      msg_out->set_abs(xbox.abs_x2, unpack::int16le(ptr+10), -32768, 32767);
      msg_out->set_abs(xbox.abs_y2, unpack::s16_invert(unpack::int16le(ptr+12)), -32768, 32767);

      return true;
    }

    if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
    {
      m_battery_status = data[4];
      log_info("battery status: {}", m_battery_status);
      return false;
    }

    if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
    {
      // Trailer after some button presses; ignore.
      return false;
    }
  }

  log_debug("unknown wireless report len={}: {}", len, raw2str(data, len));
  return false;
}

} // namespace xboxdrv

/* EOF */
