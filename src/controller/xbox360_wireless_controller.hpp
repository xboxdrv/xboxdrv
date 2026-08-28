/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#ifndef HEADER_XBOX360_WIRELESS_CONTROLLER_HPP
#define HEADER_XBOX360_WIRELESS_CONTROLLER_HPP

#include <chrono>
#include <libusb.h>
#include <memory>
#include <string>

#include <glib.h>

#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

namespace xboxdrv {

class ControllerMessage;
class Headset;
class WirelessChatpad;

class Xbox360WirelessController : public USBController
{
private:
  int  m_endpoint;
  int  m_interface;
  int  m_battery_status;
  std::string m_serial;
  std::unique_ptr<WirelessChatpad> m_chatpad;
  std::unique_ptr<Headset> m_headset;

  bool m_auto_poweroff;
  int  m_guide_poweroff_timeout_sec; // 0 = disabled
  bool m_quiet;
  bool m_pad_present;
  bool m_got_pad_report; // true once a data[1]==0x01 pad report was parsed
  std::chrono::steady_clock::time_point m_guide_down_ts;
  bool m_guide_held;
  guint m_guide_timeout_source;
  guint m_zombie_check_source;

  Xbox360DefaultNames xbox;

public:
  Xbox360WirelessController(libusb_device* dev, int controller_id,
                            bool chatpad, bool chatpad_no_init, bool chatpad_debug,
                            bool headset,
                            bool headset_debug,
                            const std::string& headset_dump,
                            const std::string& headset_play,
                            const std::string& headset_pcm,
                            const std::string& headset_wav,
                            const std::string& headset_play_wav,
                            bool headset_play_left_pack,
                            bool headset_pulse,
                            bool headset_pipewire,
                            float headset_mic_gain,
                            bool try_detach,
                            bool auto_poweroff = true,
                            int guide_poweroff_timeout_sec = 5,
                            bool quiet = false);
  ~Xbox360WirelessController() override;

  bool parse(const uint8_t* data, int len, ControllerMessage* msg_out) override;

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;
  uint8_t get_battery_status() const { return static_cast<uint8_t>(m_battery_status); }

  /** Send the wireless power-off packet (same as kernel xpad). */
  void power_off();

private:
  /** Same presence inquiry as kernel xpad_inquiry_pad_presence(). */
  void inquire_presence();

  void maybe_guide_poweroff(bool guide_down);
  void stop_guide_timeout();
  bool on_guide_timeout();
  static gboolean on_guide_timeout_wrap(gpointer data);

  void stop_zombie_check();
  bool on_zombie_check();
  static gboolean on_zombie_check_wrap(gpointer data);

  Xbox360WirelessController (const Xbox360WirelessController&);
  Xbox360WirelessController& operator= (const Xbox360WirelessController&);
};

} // namespace xboxdrv

#endif

/* EOF */
