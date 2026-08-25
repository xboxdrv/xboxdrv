/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "usb_helper.hpp"

namespace unsebu {

int usb_claim_n_detach_interface(libusb_device_handle* handle, int interface, bool try_detach)
{
  int err = libusb_claim_interface(handle, interface);

  if (err == LIBUSB_ERROR_BUSY)
  {
    if (try_detach)
    {
      err = libusb_detach_kernel_driver(handle, interface);
      if (err == LIBUSB_SUCCESS)
      {
        err = libusb_claim_interface(handle, interface);
        return err;
      }
      else
      {
        return err;
      }
    }
    else
    {
      return err;
    }
  }
  else
  {
    // success or unknown failure
    return err;
  }
}

libusb_device* usb_find_device_by_path(uint8_t busnum, uint8_t devnum)
{
  libusb_device* result = nullptr;

  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];

    if (busnum == libusb_get_bus_number(dev) &&
        devnum == libusb_get_device_address(dev))
    {
      result = dev;
      libusb_ref_device(result);
      break;
    }
  }
  libusb_free_device_list(list, 1 /* unref_devices */);

  return result;
}

} // namespace unsebu

/* EOF */
