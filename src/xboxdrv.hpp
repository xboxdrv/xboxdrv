/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#ifndef HEADER_XBOX360_HPP
#define HEADER_XBOX360_HPP

#include "xboxmsg.hpp"

class CommandLineOptions;
class uInput;
class XboxGenericController;

class Xboxdrv
{
private:
  void run_main(const CommandLineOptions& opts);
  void run_daemon(const CommandLineOptions& opts);
  void run_list_supported_devices();
  void run_help_devices();
  void run_list_controller();

  void print_info(struct usb_device* dev,
                  const XPadDevice& dev_type,
                  const CommandLineOptions& opts) const;
  void controller_loop(GamepadType type, uInput* uinput,
                       XboxGenericController* controller, 
                       const CommandLineOptions& opts);
  void apply_modifier(XboxGenericMsg& msg, int msec_delta, const CommandLineOptions& opts) const;

  bool find_controller_by_path(const char* busid, const char* devid,struct usb_device** xbox_device) const;
  void find_controller(struct usb_device*& dev,
                       XPadDevice&         dev_type,
                       const CommandLineOptions& opts) const;
  int  find_jsdev_number() const;
  int  find_evdev_number() const;
  bool find_controller_by_id(int id, int vendor_id, int product_id, struct usb_device** xbox_device) const;
  bool find_xbox360_controller(int id, struct usb_device** xbox_device, XPadDevice* type) const;

public:
  int main(int argc, char** argv);
};

#endif

/* EOF */
