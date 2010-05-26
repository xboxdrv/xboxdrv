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

#include <errno.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>

#include "xboxmsg.hpp"
#include "xbox_controller.hpp"

XboxController::XboxController(struct usb_device* dev_) :
  dev(dev_),
  handle(),
  endpoint_in(1),
  endpoint_out(2)
{
  find_endpoints();
  handle = usb_open(dev);
  if (!handle)
  {
    throw std::runtime_error("Error opening Xbox360 controller");
  }
  else
  {
    // FIXME: bInterfaceNumber shouldn't be hardcoded
    int err = usb_claim_interface(handle, 0);
    if (err != 0) 
    {
      std::ostringstream out;
      out << "Error couldn't claim the USB interface: " << strerror(-err) << std::endl
          << "Try to run 'rmmod xpad' and start xboxdrv again.";
      throw std::runtime_error(out.str());
    }
  }
}

XboxController::~XboxController()
{
  usb_release_interface(handle, 0); 
  usb_close(handle);
}

void
XboxController::find_endpoints()
{
  bool debug_print = false;

  for(struct usb_config_descriptor* config = dev->config;
      config != dev->config + dev->descriptor.bNumConfigurations;
      ++config)
  {
    if (debug_print) std::cout << "Config: " << static_cast<int>(config->bConfigurationValue) << std::endl;

    for(struct usb_interface* interface = config->interface;
        interface != config->interface + config->bNumInterfaces;
        ++interface)
    {
      for(struct usb_interface_descriptor* altsetting = interface->altsetting;
          altsetting != interface->altsetting + interface->num_altsetting;
          ++altsetting)
      {
        if (debug_print) std::cout << "  Interface: " << static_cast<int>(altsetting->bInterfaceNumber) << std::endl;
          
        for(struct usb_endpoint_descriptor* endpoint = altsetting->endpoint; 
            endpoint != altsetting->endpoint + altsetting->bNumEndpoints; 
            ++endpoint)
        {
          if (debug_print) 
            std::cout << "    Endpoint: " << int(endpoint->bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK)
                      << "(" << ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? "IN" : "OUT") << ")"
                      << std::endl;

          if (altsetting->bInterfaceClass    == USB_CLASS_VENDOR_SPEC &&
              altsetting->bInterfaceSubClass == 93 &&
              altsetting->bInterfaceProtocol == 1)
          {
            if (endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
            {
              endpoint_in = int(endpoint->bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK);
            }
            else
            {
              endpoint_out = int(endpoint->bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK);
            }
          }
        }
      }
    }
  }
}

void
XboxController::set_rumble(uint8_t left, uint8_t right)
{
  char rumblecmd[] = { 0x00, 0x06, 0x00, left, 0x00, right };
  usb_interrupt_write(handle, endpoint_out, rumblecmd, sizeof(rumblecmd), 0);
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
  int ret = usb_interrupt_read(handle, endpoint_in, reinterpret_cast<char*>(data), sizeof(data), timeout);

  if (ret == -ETIMEDOUT)
  {
    return false;
  }
  else if (ret < 0)
  { // Error
    std::ostringstream str;
    str << "USBError: " << ret << "\n" << usb_strerror();
    throw std::runtime_error(str.str());
  }
  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg.type = XBOX_MSG_XBOX;
    memcpy(&msg.xbox, data, sizeof(XboxMsg));
    return true;
  }
  return false;
}

/* EOF */
