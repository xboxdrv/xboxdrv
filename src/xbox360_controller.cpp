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

#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>
#include "xboxmsg.hpp"
#include "helper.hpp"
#include "xbox360_controller.hpp"

Xbox360Controller::Xbox360Controller(struct usb_device* dev, bool is_guitar)
  : dev(dev),
    is_guitar(is_guitar),
    endpoint_in(1),
    endpoint_out(2)
{
  find_endpoints();
  if (0)
    {
      std::cout << "EP(IN):  " << endpoint_in << std::endl;
      std::cout << "EP(OUT): " << endpoint_out << std::endl;
    }

  handle = usb_open(dev);

  if (0)
    {
      int err;
      if ((err = usb_set_configuration(handle, 0)) < 0)
        {
          std::ostringstream out;
          out << "Error set USB configuration: " << strerror(-err) << std::endl
              << "Try to run 'rmmod xpad' and start xboxdrv again.";
          throw std::runtime_error(out.str());
        }
    }

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

  if (0)
    {
      unsigned char arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 255 };
      for (int len = 3; len <= 8; ++len)
        {
          // Sending random data:
          for (int front = 0; front < 256; ++front)
            {
              for (size_t i = 0; i < sizeof(arr); ++i)
                {
                  char ledcmd[] = { front, len, arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i] }; 
                  printf("%d %d %d\n", len, front, arr[i]);
                  usb_interrupt_write(handle, endpoint_out, ledcmd, len, 0);

                  uint8_t data[32];
                  int ret = usb_interrupt_read(handle, endpoint_in, (char*)data, sizeof(data), 20);
                  print_raw_data(std::cout, data, ret);
                }
            }
        }
    }
}

Xbox360Controller::~Xbox360Controller()
{
  usb_release_interface(handle, 0); 
  usb_close(handle);
}

void
Xbox360Controller::find_endpoints()
{
  bool print_debug = false;

  if (print_debug)
    std::cout << "find_endpoints: numcfg: " << (int)dev->descriptor.bNumConfigurations << std::endl;

  for(int i = 0; i < dev->descriptor.bNumConfigurations; ++i)
    {
      if (print_debug)
        std::cout << "Configuration: " << i << std::endl;
      for(int j = 0; j < dev->config[i].bNumInterfaces; ++j)
        {
          if (print_debug)
            std::cout << "  Interface " << j << ":" << std::endl;
          for(int k = 0; k < dev->config[i].interface[j].num_altsetting; ++k)
            {
              for(int l = 0; l < dev->config[i].interface[j].altsetting[k].bNumEndpoints; ++l)
                {
                  if (dev->config[i].interface[j].altsetting[k].bInterfaceClass    == USB_CLASS_VENDOR_SPEC &&
                      dev->config[i].interface[j].altsetting[k].bInterfaceSubClass == 93 &&
                      dev->config[i].interface[j].altsetting[k].bInterfaceProtocol == 1)
                    {
                      if (dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
                        {
                          endpoint_in = int(dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK);
                        }
                      else
                        {
                          endpoint_out = int(dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK);
                        }
                    }

                  if (print_debug)
                    std::cout << "    Endpoint: " 
                              << int(dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK)
                              << ((dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? " (IN)" : " (OUT)")
                              << std::endl;
                }
            }
        }
    }

}

void
Xbox360Controller::set_rumble(uint8_t left, uint8_t right)
{
  char rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, endpoint_out, rumblecmd, sizeof(rumblecmd), 0);
}

void
Xbox360Controller::set_led(uint8_t status)
{
  char ledcmd[] = { 0x01, 0x03, status }; 
  usb_interrupt_write(handle, endpoint_out, ledcmd, sizeof(ledcmd), 0);
}

bool
Xbox360Controller::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  uint8_t data[32];
  int ret = usb_interrupt_read(handle, endpoint_in, (char*)data, sizeof(data), timeout);

  if (ret == -ETIMEDOUT)
    {
      return false;
    }
  else if (ret < 0)
    { // Error
      std::ostringstream str;
      str << "Xbox360Controller: USBError: " << ret << "\n" << usb_strerror();
      throw std::runtime_error(str.str());
    }
  else if (ret == 0)
    {
      if (verbose)
        {
          std::cout << "zero length read" << std::endl;
          // happens with the Xbox360 controller every now and then, just
          // ignore, seems harmless, so just ignore
        }
    }
  else if (ret == 3 && data[0] == 0x01 && data[1] == 0x03)
    { 
      if (verbose)
        {
          std::cout << "Xbox360Controller: LED Status: " << int(data[2]) << std::endl;
        }
    }
  else if (ret == 3 && data[0] == 0x03 && data[1] == 0x03)
    { 
      if (verbose)
        {
          // data[2] == 0x00 means that rumble is disabled
          // data[2] == 0x01 unknown, but rumble works
          // data[2] == 0x02 unknown, but rumble works
          // data[2] == 0x03 is default with rumble enabled
          std::cout << "Xbox360Controller: Rumble Status: " << int(data[2]) << std::endl;
        }
    }
  else if (ret == 3 && data[0] == 0x08 && data[1] == 0x03)
    {
      if (data[2] == 0x00)
        std::cout << "Headset: none";
      else if (data[2] == 0x02)
        std::cout << "Headset: none";
    }
  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
    {
      if (is_guitar)
        {
          msg.type   = XBOX_MSG_XBOX360_GUITAR;
          msg.guitar = *reinterpret_cast<Xbox360GuitarMsg*>(data);
        }
      else
        {
          msg.type    = XBOX_MSG_XBOX360;
          msg.xbox360 = *reinterpret_cast<Xbox360Msg*>(data);
        }
      return true;
    }
  else
    {
      std::cout << "Unknown: ";
      print_raw_data(std::cout, data, ret);
    }

  return false;
}

/* EOF */
