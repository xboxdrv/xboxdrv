/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

struct XboxGenericMsg;
struct XPadDevice;

class Xbox360WirelessController : public USBController
{
private:
  bool m_active;
  int  m_endpoint;
  int  m_interface;
  int  m_battery_status;
  std::string m_serial;
  int m_led_status;

  boost::function<void ()> m_activation_cb;

public:
  Xbox360WirelessController(libusb_device* dev, int controller_id, bool try_detach);
  virtual ~Xbox360WirelessController();

  bool parse(uint8_t* data, int len, XboxGenericMsg* msg_out);

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);
  uint8_t get_battery_status() const;
  bool is_active() const { return m_active; }
  void set_activation_cb(const boost::function<void ()> callback);
  
private:
  void set_active(bool v);

private:
  Xbox360WirelessController (const Xbox360WirelessController&);
  Xbox360WirelessController& operator= (const Xbox360WirelessController&);
};

#endif

/* EOF */
