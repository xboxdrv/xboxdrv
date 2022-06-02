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

#include "usb_subsystem.hpp"

#include <stdexcept>
#include <fmt/format.h>

#include "options.hpp"
#include "raise_exception.hpp"
#include "usb_gsource.hpp"
#include "usb_helper.hpp"
#include "util/string.hpp"

namespace xboxdrv {

USBSubsystem::USBSubsystem() :
  m_usb_gsource()
{
  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_init() failed: " << libusb_strerror(ret));
  }

  m_usb_gsource.reset(new USBGSource);
  m_usb_gsource->attach(NULL);
}

USBSubsystem::~USBSubsystem()
{
  m_usb_gsource.reset();
  libusb_exit(NULL);
}

void
USBSubsystem::find_controller(libusb_device** dev, XPadDevice& dev_type, Options const& opts)
{
  if (opts.busid[0] != '\0' && opts.devid[0] != '\0')
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      throw std::runtime_error("--device-by-path BUS:DEV option must be used in combination with --type TYPE option");
    }
    else
    {
      if (!find_controller_by_path(opts.busid, opts.devid, dev))
      {
        raise_exception(std::runtime_error, "couldn't find device " << opts.busid << ":" << opts.devid);
      }
      else
      {
        dev_type.type      = opts.gamepad_type;
        dev_type.name      = "unknown";
        libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(*dev, &desc) == LIBUSB_SUCCESS)
        {
          dev_type.idVendor  = desc.idVendor;
          dev_type.idProduct = desc.idProduct;
        }
      }
    }
  }
  else if (opts.vendor_id != -1 && opts.product_id != -1)
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      throw std::runtime_error("--device-by-id VENDOR:PRODUCT option must be used in combination with --type TYPE option");
    }
    else
    {
      if (!find_controller_by_id(opts.controller_id, opts.vendor_id, opts.product_id, dev))
      {
        raise_exception(std::runtime_error, "couldn't find device with "
                        << fmt::format("{:04x}:{:04x}", opts.vendor_id, opts.product_id));
      }
      else
      {
        dev_type.type = opts.gamepad_type;
        dev_type.idVendor  = static_cast<uint16_t>(opts.vendor_id);
        dev_type.idProduct = static_cast<uint16_t>(opts.product_id);
        dev_type.name = "unknown";
      }
    }
  }
  else
  {
    if (!find_xbox360_controller(opts.controller_id, dev, &dev_type))
    {
      throw std::runtime_error("No Xbox or Xbox360 controller found");
    }
  }
}

bool
USBSubsystem::find_controller_by_path(std::string const& busid_str, std::string const& devid_str,
                                 libusb_device** xbox_device)
{
  int busid = str2int(busid_str);
  int devid = str2int(devid_str);

  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];

    if (libusb_get_bus_number(dev)     == busid &&
        libusb_get_device_address(dev) == devid)
    {
      *xbox_device = dev;

      // incrementing ref count, user must call unref
      libusb_ref_device(*xbox_device);
      libusb_free_device_list(list, 1 /* unref_devices */);
      return true;
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

bool
USBSubsystem::find_controller_by_id(int id, int vendor_id, int product_id, libusb_device** xbox_device)
{
  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  int id_count = 0;
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      log_warn("libusb_get_device_descriptor() failed: {}", libusb_strerror(ret));
    }
    else
    {
      if (desc.idVendor  == vendor_id &&
          desc.idProduct == product_id)
      {
        if (id_count == id)
        {
          *xbox_device = dev;
          // increment ref count, user must free the device
          libusb_ref_device(*xbox_device);
          libusb_free_device_list(list, 1 /* unref_devices */);
          return true;
        }
        else
        {
          id_count += 1;
        }
      }
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

bool
USBSubsystem::find_xbox360_controller(int id, libusb_device** xbox_device, XPadDevice* type)
{
  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  int id_count = 0;
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      log_warn("libusb_get_device_descriptor() failed: {}", libusb_strerror(ret));
    }
    else
    {
      for(int i = 0; i < xpad_devices_count; ++i)
      {
        if (desc.idVendor  == xpad_devices[i].idVendor &&
            desc.idProduct == xpad_devices[i].idProduct)
        {
          if (id_count == id)
          {
            *xbox_device = dev;
            *type        = xpad_devices[i];
            // increment ref count, user must free the device
            libusb_ref_device(*xbox_device);
            libusb_free_device_list(list, 1 /* unref_devices */);
            return true;
          }
          else
          {
            id_count += 1;
          }
        }
      }
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

} // namespace xboxdrv

/* EOF */
