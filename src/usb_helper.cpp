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

int usb_claim_n_detach_interface(libusb_device_handle* handle, int interface, bool try_detach)
{
  int ret = libusb_claim_interface(handle, interface);

  if (ret == LIBUSB_ERROR_BUSY)
  {
    if (try_detach)
    {
      ret = libusb_detach_kernel_driver(handle, interface);
      if (ret == LIBUSB_SUCCESS)
      {
        ret = libusb_claim_interface(handle, interface);
        return ret;
      }
      else
      {
        return ret;
      }
    }
    else
    {
      return ret;
    }
  }
  else
  {
    // success or unknown failure
    return ret;
  }
}

const char* usb_strerror(int err)
{
  switch(err)
  {
    case LIBUSB_SUCCESS: return "LIBUSB_SUCCESS";
    case LIBUSB_ERROR_IO: return "LIBUSB_ERROR_IO";
    case LIBUSB_ERROR_INVALID_PARAM: return "LIBUSB_ERROR_INVALID_PARAM";
    case LIBUSB_ERROR_ACCESS: return "LIBUSB_ERROR_ACCESS";
    case LIBUSB_ERROR_NO_DEVICE: return "LIBUSB_ERROR_NO_DEVICE";
    case LIBUSB_ERROR_NOT_FOUND: return "LIBUSB_ERROR_NOT_FOUND";
    case LIBUSB_ERROR_BUSY: return "LIBUSB_ERROR_BUSY";
    case LIBUSB_ERROR_TIMEOUT: return "LIBUSB_ERROR_TIMEOUT";
    case LIBUSB_ERROR_OVERFLOW: return "LIBUSB_ERROR_OVERFLOW";
    case LIBUSB_ERROR_PIPE: return "LIBUSB_ERROR_PIPE";
    case LIBUSB_ERROR_INTERRUPTED: return "LIBUSB_ERROR_INTERRUPTED";
    case LIBUSB_ERROR_NO_MEM: return "LIBUSB_ERROR_NO_MEM";
    case LIBUSB_ERROR_NOT_SUPPORTED: return "LIBUSB_ERROR_NOT_SUPPORTED";
    case LIBUSB_ERROR_OTHER: return "LIBUSB_ERROR_OTHER";
    default: return "<unknown libusb error code>";
  }
}

const char* usb_transfer_strerror(libusb_transfer_status err)
{
  switch(err)
  {
    case LIBUSB_TRANSFER_COMPLETED: return "LIBUSB_TRANSFER_COMPLETED";
    case LIBUSB_TRANSFER_ERROR: return "LIBUSB_TRANSFER_ERROR";
    case LIBUSB_TRANSFER_TIMED_OUT: return "LIBUSB_TRANSFER_TIMED_OUT";
    case LIBUSB_TRANSFER_CANCELLED: return "LIBUSB_TRANSFER_CANCELLED";
    case LIBUSB_TRANSFER_STALL: return "LIBUSB_TRANSFER_STALL";
    case LIBUSB_TRANSFER_NO_DEVICE: return "LIBUSB_TRANSFER_NO_DEVICE";
    case LIBUSB_TRANSFER_OVERFLOW: return "LIBUSB_TRANSFER_OVERFLOW";
    default: return "<unknown libusb transfer error code>";
  }
}

libusb_device* usb_find_device_by_path(uint8_t busnum, uint8_t devnum)
{
  libusb_device* ret_device = nullptr;

  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];

    if (busnum == libusb_get_bus_number(dev) &&
        devnum == libusb_get_device_address(dev))
    {
      ret_device = dev;
      libusb_ref_device(ret_device);
      break;
    }
  }
  libusb_free_device_list(list, 1 /* unref_devices */);

  return ret_device;
}

/* EOF */
