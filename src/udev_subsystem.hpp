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

#ifndef HEADER_XBOXDRV_UDEV_SUBSYSTEM_HPP
#define HEADER_XBOXDRV_UDEV_SUBSYSTEM_HPP

#include <boost/function.hpp>
#include <libudev.h>
#include <glib.h>

class UdevSubsystem
{
private:
  struct udev* m_udev;
  struct udev_monitor* m_monitor;

  boost::function<void (udev_device*)> m_process_match_cb;

public:
  UdevSubsystem();
  ~UdevSubsystem();

  void set_device_callback(const boost::function<void (udev_device*)>& process_match_cb);
  void enumerate_udev_devices();
  void print_info(udev_device* device);

private:
  bool on_udev_data(GIOChannel* channel, GIOCondition condition);

  static gboolean on_udev_data_wrap(GIOChannel* channel, GIOCondition condition, gpointer data) {
    return static_cast<UdevSubsystem*>(data)->on_udev_data(channel, condition);
  }

private:
  UdevSubsystem(const UdevSubsystem&);
  UdevSubsystem& operator=(const UdevSubsystem&);
};

#endif

/* EOF */
