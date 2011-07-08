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

#ifndef HEADER_XBOXDRV_XBOXDRV_HPP
#define HEADER_XBOXDRV_XBOXDRV_HPP

#include <libusb.h>

#include "xboxmsg.hpp"

struct XPadDevice;
class Options;
class Controller;

class Xboxdrv
{
private:
  void run_main(const Options& opts);
  void run_daemon(const Options& opts);
  void run_list_supported_devices();
  void run_list_supported_devices_xpad();
  void run_list_enums(uint32_t enums);
  void run_help_devices();
  void run_list_controller();

  void print_copyright() const;

public:
  Xboxdrv();
  ~Xboxdrv();

  int main(int argc, char** argv);

private:
  void set_scheduling(const Options& opts);
};

#endif

/* EOF */
