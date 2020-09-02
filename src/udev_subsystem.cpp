/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "udev_subsystem.hpp"

#include <assert.h>
#include <stdexcept>

#include "raise_exception.hpp"

UdevSubsystem::UdevSubsystem() :
  m_udev(),
  m_monitor(),
  m_process_match_cb()
{
  m_udev = udev_new();
  if (!m_udev)
  {
    raise_exception(std::runtime_error, "udev init failure");
  }
}

UdevSubsystem::~UdevSubsystem()
{
  if (m_monitor)
  {
    udev_monitor_unref(m_monitor);
  }
  udev_unref(m_udev);
}

void
UdevSubsystem::set_device_callback(const boost::function<void (udev_device*)>& process_match_cb)
{
  assert(process_match_cb);
  assert(!m_process_match_cb);

  m_process_match_cb = process_match_cb;

  // Setup udev monitor and enumerate
  m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", "usb_device");
  udev_monitor_enable_receiving(m_monitor);

  // FIXME: won't we get devices twice that have been plugged in at
  // this point? once from the enumeration, once from the monitor
  enumerate_udev_devices();

  GIOChannel* udev_channel = g_io_channel_unix_new(udev_monitor_get_fd(m_monitor));
  g_io_add_watch(udev_channel,
                 static_cast<GIOCondition>(G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP),
                 &UdevSubsystem::on_udev_data_wrap, this);
  g_io_channel_unref(udev_channel);
}

void
UdevSubsystem::enumerate_udev_devices()
{
  assert(m_process_match_cb);

  // Enumerate over all devices already connected to the computer
  struct udev_enumerate* enumerate = udev_enumerate_new(m_udev);
  assert(enumerate);

  udev_enumerate_add_match_subsystem(enumerate, "usb");
  // not available yet: udev_enumerate_add_match_is_initialized(enumerate);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry* devices;
  struct udev_list_entry* dev_list_entry;

  devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    // name is path, value is NULL
    const char* path = udev_list_entry_get_name(dev_list_entry);

    struct udev_device* device = udev_device_new_from_syspath(m_udev, path);

    // manually filter for devtype, as udev enumerate can't do it by itself
    const char* devtype = udev_device_get_devtype(device);
    if (devtype && strcmp(devtype, "usb_device") == 0)
    {
      m_process_match_cb(device);
    }
    udev_device_unref(device);
  }
  udev_enumerate_unref(enumerate);
}

bool
UdevSubsystem::on_udev_data(GIOChannel* channel, GIOCondition condition)
{
  if (condition == G_IO_OUT)
  {
    log_error("data can be written");
  }
  else if (condition == G_IO_PRI)
  {
    log_error("data can be read");
  }
  else if (condition == G_IO_ERR)
  {
    log_error("data error");
  }
  else if (condition != G_IO_IN)
  {
    log_error("unknown condition: " << condition);
  }
  else
  {
    log_info("trying to read data from udev");

    log_info("trying to read data from udev monitor");
    struct udev_device* device = udev_monitor_receive_device(m_monitor);
    log_info("got data from udev monitor");

    if (!device)
    {
      // seem to be normal, do we get this when the given device is filtered out?
      log_debug("udev device couldn't be read: " << device);
    }
    else
    {
      const char* action = udev_device_get_action(device);

      if (g_logger.get_log_level() >= Logger::kDebug)
      {
        print_info(device);
      }

      if (action && strcmp(action, "add") == 0)
      {
        m_process_match_cb(device);
      }

      udev_device_unref(device);
    }
  }

  return true;
}

void
UdevSubsystem::print_info(udev_device* device)
{
  log_debug("/---------------------------------------------");
  log_debug("devpath: " << udev_device_get_devpath(device));

  if (udev_device_get_action(device))
    log_debug("action: " << udev_device_get_action(device));
  //log_debug("init: " << udev_device_get_is_initialized(device));

  if (udev_device_get_subsystem(device))
    log_debug("subsystem: " << udev_device_get_subsystem(device));

  if (udev_device_get_devtype(device))
    log_debug("devtype:   " << udev_device_get_devtype(device));

  if (udev_device_get_syspath(device))
    log_debug("syspath:   " << udev_device_get_syspath(device));

  if (udev_device_get_sysname(device))
    log_debug("sysname:   " << udev_device_get_sysname(device));

  if (udev_device_get_sysnum(device))
    log_debug("sysnum:    " << udev_device_get_sysnum(device));

  if (udev_device_get_devnode(device))
    log_debug("devnode:   " << udev_device_get_devnode(device));

  if (udev_device_get_driver(device))
    log_debug("driver:    " << udev_device_get_driver(device));

  if (udev_device_get_action(device))
    log_debug("action:    " << udev_device_get_action(device));

  //udev_device_get_sysattr_value(device, "busnum");
  //udev_device_get_sysattr_value(device, "devnum");

#if 0
  // FIXME: only works with newer versions of libudev
  {
    log_debug("list: ");
    struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {
      log_debug("  "
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }

  {
    log_debug("properties: ");
    struct udev_list_entry* it = udev_device_get_properties_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {
      log_debug("  "
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }

  {
    log_debug("devlist: ");
    struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {
      log_debug("  "
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }
#endif

  log_debug("\\----------------------------------------------");
}

/* EOF */
