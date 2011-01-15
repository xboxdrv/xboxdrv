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

#ifndef HEADER_XBOXDRV_XBOXDRV_DAEMON_HPP
#define HEADER_XBOXDRV_XBOXDRV_DAEMON_HPP

#include <libudev.h>
#include <stdint.h>
#include <vector>

class Options;
class uInput;
struct XPadDevice;
class XboxdrvThread;

class XboxdrvDaemon
{
private:
  struct udev* m_udev;
  struct udev_monitor* m_monitor;
  typedef std::vector<XboxdrvThread*> Threads;
  Threads m_threads;

public:
  XboxdrvDaemon();
  ~XboxdrvDaemon();

  void run(const Options& opts);

private:
  void process_match(const Options& opts, uInput* uinput, struct udev_device* device);
  void print_info(struct udev_device* device);
  void launch_xboxdrv(uInput* uinput,
                      const XPadDevice& dev_type, const Options& opts, 
                      uint8_t busnum, uint8_t devnum);
  
private:
  XboxdrvDaemon(const XboxdrvDaemon&);
  XboxdrvDaemon& operator=(const XboxdrvDaemon&);
};

#endif

/* EOF */
