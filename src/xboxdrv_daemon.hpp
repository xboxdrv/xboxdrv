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

#include "controller_config_set.hpp"

class Options;
class UInput;
struct XPadDevice;
class XboxdrvThread;

class XboxdrvDaemon
{
private:
  struct udev* m_udev;
  struct udev_monitor* m_monitor;

  struct ControllerSlot
  {
    ControllerConfigSetPtr config;
    XboxdrvThread* thread;
    
    ControllerSlot() :
      config(),
      thread(0)
    {}

    ControllerSlot(ControllerConfigSetPtr config_,
                   XboxdrvThread* thread_ = 0) :
      config(config_),
      thread(thread_)
    {}

    ControllerSlot(const ControllerSlot& rhs) :
      config(rhs.config),
      thread(rhs.thread)
    {}

    ControllerSlot& operator=(const ControllerSlot& rhs)
    {
      if (&rhs != this)
      {
        config = rhs.config;
        thread = rhs.thread;
      }
      return *this;
    }
  };
  
  typedef std::vector<ControllerSlot> ControllerSlots;
  ControllerSlots m_controller_slots;

public:
  XboxdrvDaemon();
  ~XboxdrvDaemon();

  void run(const Options& opts);

private:
  void run_real(const Options& opts);

  void cleanup_threads();
  void process_match(const Options& opts, UInput* uinput, struct udev_device* device);
  void print_info(struct udev_device* device);
  void launch_xboxdrv(UInput* uinput,
                      const XPadDevice& dev_type, const Options& opts, 
                      uint8_t busnum, uint8_t devnum);
  int get_free_slot_count() const;
  
private:
  XboxdrvDaemon(const XboxdrvDaemon&);
  XboxdrvDaemon& operator=(const XboxdrvDaemon&);
};

#endif

/* EOF */
