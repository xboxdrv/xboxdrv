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
#include "wakeup_pipe.hpp"

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
 
  typedef std::vector<ControllerSlotPtr> ControllerSlots;
  ControllerSlots m_controller_slots;
  
  typedef std::vector<XboxdrvThreadPtr> Threads;
  Threads m_inactive_threads;

  std::auto_ptr<UInput> m_uinput;

  WakeupPipe m_wakeup_pipe;

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

  std::vector<ControllerSlotPtr> find_compatible_slots(udev_device* dev);
  ControllerSlotPtr find_free_slot(udev_device* dev);
  ControllerSlotPtr find_free_slot(XboxdrvThreadPtr thread);

  void enumerate_udev_devices(const Options& opts);
  void cleanup_threads();
  void process_match(const Options& opts, struct udev_device* device);
  void print_info(struct udev_device* device);
  void launch_xboxdrv(udev_device* dev,
                      const XPadDevice& dev_type, const Options& opts, 
                      uint8_t busnum, uint8_t devnum,
                      ControllerSlotPtr slot);
  int get_free_slot_count() const;
  void check_thread_status();
  
  void connect(ControllerSlotPtr slot, XboxdrvThreadPtr thread);
  XboxdrvThreadPtr disconnect(ControllerSlotPtr slot);

  void on_connect(ControllerSlotPtr slot);
  void on_disconnect(ControllerSlotPtr slot);

  void wakeup();
  
private:
  XboxdrvDaemon(const XboxdrvDaemon&);
  XboxdrvDaemon& operator=(const XboxdrvDaemon&);
};

#endif

/* EOF */
