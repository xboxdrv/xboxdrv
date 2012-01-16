/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_CONTROLLER_HAMA_CRUX_CONTROLLER_HPP
#define HEADER_XBOXDRV_CONTROLLER_HAMA_CRUX_CONTROLLER_HPP

#include <libusb.h>

#include "controller_message.hpp"
#include "controller/usb_controller.hpp"
#include "xboxmsg.hpp"

class ControllerMessageDescriptor;

struct HamaCruxNames
{
  int crouch, run, talk;
  int esc, pause, option; 
  int quickload, quicksave, print;
  int n1, n2, n3, n4, n5, n6;
  int n7, n8, n9, n10, n11;
  int up, down, left, right, q, e;
  int c1, tab, c2, c3, c4, reload, use, c8, p2, n;
  int c5, c6, c7, p1, space;

  HamaCruxNames(ControllerMessageDescriptor& msg_desc);
};

class HamaCruxController : public USBController
{
private:
  int m_interface;
  int m_endpoint;

  HamaCruxNames m_names;

public:
  HamaCruxController(libusb_device* dev, bool try_detach);
  ~HamaCruxController();

  void set_rumble_real(uint8_t left, uint8_t right);
  void set_led_real(uint8_t status);
  bool parse(const uint8_t* data, int len, ControllerMessage* msg_out);

private:
  void print(libusb_config_descriptor* config, std::ostream& out) const;

private:
  HamaCruxController(const HamaCruxController&);
  HamaCruxController& operator=(const HamaCruxController&);
};

#endif

/* EOF */
