/*
**  Xbox/Xbox360/XboxOne USB Gamepad Userspace Driver
**  Copyright (C) 2015 Andrey Turkin <andrey.turkin@gmail.com>
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

#ifndef HEADER_XBOXONE_WIRELESS_CONTROLLER_HPP
#define HEADER_XBOXONE_WIRELESS_CONTROLLER_HPP

#include <libusb.h>
#include <string>

#include "controller/usb_controller.hpp"
#include "xbox360_default_names.hpp"

struct XPadDevice;

class XboxOneWirelessController : public USBController
{
private:
  XPadDevice*        dev_type;

  int  endpoint_in;
  int  endpoint_out;
  uint8_t m_sequence;

  int  m_battery_status;
  std::string m_serial;

  Xbox360DefaultNames xbox;

public:
  XboxOneWirelessController(libusb_device* dev, bool try_detach);
  ~XboxOneWirelessController();

  bool parse(const uint8_t* data, int len, ControllerMessage* msg_out) override;

  void set_rumble_real(uint8_t left, uint8_t right) override;
  void set_led_real(uint8_t status) override;
  uint8_t get_battery_status() const;

private:
  void submit_command(uint8_t cmd1, uint8_t cmd2, const uint8_t* data, int len);
  XboxOneWirelessController (const XboxOneWirelessController&);
  XboxOneWirelessController& operator= (const XboxOneWirelessController&);
};

#endif

/* EOF */
