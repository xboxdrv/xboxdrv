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

#include "controller_slot_config.hpp"
#include "controller_slot.hpp"

class Options;
class UInput;
class XboxdrvThread;
struct XPadDevice;

class XboxdrvDaemon
{
private:
  const Options& m_opts;
  struct udev* m_udev;
  struct udev_monitor* m_monitor;

 
  typedef std::vector<ControllerSlot> ControllerSlots;
  ControllerSlots m_controller_slots;

  std::auto_ptr<UInput> m_uinput;

public:
  XboxdrvDaemon(const Options& opts);
  ~XboxdrvDaemon();

  void run(const Options& opts);

private:
  void create_pid_file(const Options& opts);
  void init_uinput(const Options& opts);
  void init_udev();
  void init_udev_monitor(const Options& opts);

  void run_loop(const Options& opts);

  ControllerSlot* find_free_slot(udev_device* dev);

  void cleanup_threads();
  void process_match(const Options& opts, struct udev_device* device);
  void print_info(struct udev_device* device);
  void launch_xboxdrv(const XPadDevice& dev_type, const Options& opts, 
                      uint8_t busnum, uint8_t devnum,
                      ControllerSlot& slot);
  int get_free_slot_count() const;
  
  void on_connect(const ControllerSlot& slot);
  void on_disconnect(const ControllerSlot& slot);

private:
  XboxdrvDaemon(const XboxdrvDaemon&);
  XboxdrvDaemon& operator=(const XboxdrvDaemon&);
};

#endif

/* EOF */
