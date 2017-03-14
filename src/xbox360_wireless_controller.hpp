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

#ifndef HEADER_XBOX360_WIRELESS_CONTROLLER_HPP
#define HEADER_XBOX360_WIRELESS_CONTROLLER_HPP

#include <libusb.h>
#include <string>

#include "usb_controller.hpp"
#include "linux_uinput.hpp"

struct XboxGenericMsg;
struct XPadDevice;

class Xbox360WirelessController : public USBController
{
private:
  int  m_endpoint;
  int  m_interface;
  int  m_battery_status;
  std::string m_serial;
  bool m_chatpad;
  time_t m_chatpad_next;
  int m_chatpad_timeout;
  std::auto_ptr<LinuxUinput> m_uinput;
  uint32_t m_chatpad_lastpacket;
  uint8_t m_chatpad_laststroke[3];
  uint8_t m_chatpad_keymap[256];

public:
  Xbox360WirelessController(libusb_device* dev, bool chatpad, int controller_id, bool try_detach);
  virtual ~Xbox360WirelessController();

  bool parse(uint8_t* data, int len, XboxGenericMsg* msg_out);

  void set_rumble_real(uint8_t left, uint8_t right);
  void set_led_real(uint8_t status);
  uint8_t get_battery_status() const;

private:
  Xbox360WirelessController (const Xbox360WirelessController&);
  Xbox360WirelessController& operator= (const Xbox360WirelessController&);
  void chatpad_send(uint8_t cmd);
};

#endif

/* EOF */
