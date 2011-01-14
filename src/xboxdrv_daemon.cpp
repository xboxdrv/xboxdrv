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

#include "xboxdrv_daemon.hpp"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>

XboxdrvDaemon::XboxdrvDaemon() :
  m_udev(0),
  m_monitor(0)
{
  m_udev = udev_new();
    
  if (!m_udev)
  {
    throw std::runtime_error("udev init failure");
  }
  else
  {
    // FIXME: add enumerate here, see libudev example on how to avoid
    // race condition
    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    udev_monitor_enable_receiving(m_monitor);

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", "usb_device");
  }
}

XboxdrvDaemon::~XboxdrvDaemon()
{
  udev_monitor_unref(m_monitor);
  udev_unref(m_udev);
}

void
XboxdrvDaemon::run()
{
  while(true)
  {
    struct udev_device* device = udev_monitor_receive_device(m_monitor);

    if (!device)
    {
      // seem to be normal, do we get this when the given device is filtered out?
      std::cout << "udev device couldn't be read: " << device << std::endl;
    }
    else
    {
      const char* action = udev_device_get_action(device);

      if (action && strcmp(action, "add") == 0)
      {
        // 1) Match vendor/product against the xpad list
        // value = udev_device_get_property_value(device, "ID_VENDOR_ID"); // 045e
        // value = udev_device_get_property_value(device, "ID_MODEL_ID");  // 028e
        // value = udev_device_get_property_value(device, "ID_REVISION");  // 0110 aka bcd
        // PRODUCT = "45e/28e/110"

        const char* product_str = udev_device_get_property_value(device, "PRODUCT");
        if (product_str)
        {
          unsigned int vendor = 0;
          unsigned int product = 0; 
          unsigned int bcd = 0;
          if (sscanf(product_str, "%x/%x/%x", &vendor, &product, &bcd) != 3)
          {
            std::cout << "[XboxdrvDaemon] couldn't parse PRODUCT = " << product_str << std::endl;
          }
          else
          {
            std::cout << "Product parse: " 
                      << boost::format("%03x/%03x/%03x == %s") % vendor % product % bcd % product_str
                      << std::endl;

            // 2) Get busnum and devnum        
            // FIXME: are those dec or hex?
            const char* busnum_str = udev_device_get_property_value(device, "BUSNUM");
            const char* devnum_str = udev_device_get_property_value(device, "DEVNUM");

            if (busnum_str && devnum_str)
            {
              try 
              {
              // 3) Launch thread to handle the device
              launch_xboxdrv(boost::lexical_cast<int>(busnum_str),
                             boost::lexical_cast<int>(devnum_str));
              }
              catch(const std::exception& err)
              {
                std::cout << "[XboxdrvDaemon] child thread lauch failure: " << err.what() << std::endl;
              }
            }
          }
        }
      }
    }
    udev_device_unref(device);
  }
}

void
XboxdrvDaemon::print_info(struct udev_device* device)
{
  std::cout << "/---------------------------------------------" << std::endl;
  std::cout << "devpath: " << udev_device_get_devpath(device) << std::endl;
  std::cout << "action: " << udev_device_get_action(device) << std::endl;
  //std::cout << "init: " << udev_device_get_is_initialized(device) << std::endl;

  if (strcmp(udev_device_get_action(device), "add") == 0)
  {
    if (udev_device_get_subsystem(device))
      std::cout << "subsystem: " << udev_device_get_subsystem(device) << std::endl;

    if (udev_device_get_devtype(device))
      std::cout << "devtype:   " << udev_device_get_devtype(device) << std::endl;

    if (udev_device_get_syspath(device))
      std::cout << "syspath:   " << udev_device_get_syspath(device) << std::endl;

    if (udev_device_get_sysname(device))
      std::cout << "sysname:   " << udev_device_get_sysname(device) << std::endl;

    if (udev_device_get_sysnum(device))
      std::cout << "sysnum:    " << udev_device_get_sysnum(device) << std::endl;

    if (udev_device_get_devnode(device))
      std::cout << "devnode:   " << udev_device_get_devnode(device) << std::endl;

    if (udev_device_get_driver(device))
      std::cout << "driver:    " << udev_device_get_driver(device) << std::endl;

    if (udev_device_get_action(device))
      std::cout << "action:    " << udev_device_get_action(device) << std::endl;
          
    udev_device_get_sysattr_value(device, "busnum");
    udev_device_get_sysattr_value(device, "devnum");

    {
      std::cout << "list: " << std::endl;
      struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
          
    {
      std::cout << "properties: " << std::endl;
      struct udev_list_entry* it = udev_device_get_properties_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
          
    {
      std::cout << "devlist: " << std::endl;
      struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
  }
  std::cout << "\\----------------------------------------------" << std::endl;
}

void
XboxdrvDaemon::launch_xboxdrv(uint8_t busnum, uint8_t devnum)
{
  std::cout << "[XboxdrvDaemon] launching " << boost::format("%03d:%03d") % busnum % devnum << std::endl;
}

/* EOF */
