/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
**  Copyright (C) 2016 James Le Cuirot <chewi@gentoo.org>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Experimental Steam Controller backend (PR #222). Prefer kernel
**  hid-steam or Steam Input for everyday use.
*/

#ifndef HEADER_XBOXDRV_STEAM_CONTROLLER_HPP
#define HEADER_XBOXDRV_STEAM_CONTROLLER_HPP

#include <libusb.h>
#include <stdint.h>

#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

namespace xboxdrv {

class SteamController : public USBController
{
private:
  uint8_t m_controller_id;
  Xbox360DefaultNames xbox;

public:
  /** controller_id: 0 = wired USB; 1..4 = wireless dongle slots. */
  SteamController(libusb_device* dev, uint8_t controller_id, bool try_detach);
  ~SteamController();

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;

  bool parse(uint8_t const* data, int len, ControllerMessage* msg_out) override;

private:
  void send_usb_control(uint8_t* cmd);

  SteamController(const SteamController&);
  SteamController& operator=(const SteamController&);
};

} // namespace xboxdrv

#endif

/* EOF */
