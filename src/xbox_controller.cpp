/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xbox_controller.hpp"

#include <sstream>
#include <stdexcept>
#include <string.h>

#include "log.hpp"
#include "usb_helper.hpp"
#include "raise_exception.hpp"
#include "xboxmsg.hpp"

XboxController::XboxController(libusb_device* dev_, bool try_detach) :
  dev(dev_),
  handle(),
  endpoint_in(1),
  endpoint_out(2)
{
  find_endpoints();
  int ret = libusb_open(dev, &handle);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_open() failed: " << usb_strerror(ret));
  }
  else
  {
    // FIXME: bInterfaceNumber shouldn't be hardcoded
    int err = usb_claim_n_detach_interface(handle, 0, try_detach);
    if (err != 0) 
    {
      raise_exception(std::runtime_error,
                      "Error couldn't claim the USB interface: " << strerror(-err) << std::endl <<
                      "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.");
    }
  }
}

XboxController::~XboxController()
{
  libusb_release_interface(handle, 0); 
  libusb_close(handle);
}

void
XboxController::find_endpoints()
{
  libusb_config_descriptor* config;
  int ret = libusb_get_config_descriptor(dev, 0 /* config_index */, &config);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << usb_strerror(ret));
  }

  // FIXME: no need to search all interfaces, could just check the one we acutally use
  for(const libusb_interface* interface = config->interface;
      interface != config->interface + config->bNumInterfaces;
      ++interface)
  {
    for(const libusb_interface_descriptor* altsetting = interface->altsetting;
        altsetting != interface->altsetting + interface->num_altsetting;
        ++altsetting)
    {
      log_debug("  Interface: " << static_cast<int>(altsetting->bInterfaceNumber));
          
      for(const libusb_endpoint_descriptor* endpoint = altsetting->endpoint; 
          endpoint != altsetting->endpoint + altsetting->bNumEndpoints; 
          ++endpoint)
      {
        log_debug("    Endpoint: " << int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK) <<
                  "(" << ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? "IN" : "OUT") << ")");
                  
        if (altsetting->bInterfaceClass    == 88 &&
            altsetting->bInterfaceSubClass == 66 &&
            altsetting->bInterfaceProtocol == 0)
        {
          if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
          {
            endpoint_in = int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK);
          }
          else
          {
            endpoint_out = int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK);
          }
        }
      }
    }
  }

  libusb_free_config_descriptor(config);
}

void
XboxController::set_rumble(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x06, 0x00, left, 0x00, right };
  int transferred = 0;
  int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                      rumblecmd, sizeof(rumblecmd), &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

void
XboxController::set_led(uint8_t status)
{
  // Controller doesn't have a LED
}

bool
XboxController::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  // FIXME: Add tracking for duplicate data packages (send by logitech controller)
  uint8_t data[32];
  int len = 0;
  int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_IN | endpoint_in,
                                      data, sizeof(data), &len, timeout);

  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
  else if (len == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg.type = XBOX_MSG_XBOX;
    memcpy(&msg.xbox, data, sizeof(XboxMsg));
    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
