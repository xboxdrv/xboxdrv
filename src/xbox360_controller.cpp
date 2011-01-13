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
#include <stdio.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
#include <boost/format.hpp>

#include "chatpad.hpp"
#include "headset.hpp"
#include "helper.hpp"
#include "options.hpp"
#include "usb_helper.hpp"
#include "usb_read_thread.hpp"
#include "xbox360_controller.hpp"
#include "xboxmsg.hpp"

Xbox360Controller::Xbox360Controller(libusb_device* dev_, 
                                     bool chatpad, bool chatpad_no_init, bool chatpad_debug, 
                                     bool headset, 
                                     bool headset_debug, 
                                     const std::string& headset_dump,
                                     const std::string& headset_play,
                                     bool try_detach) :
  dev(dev_),
  dev_type(),
  handle(),
  endpoint_in(1),
  endpoint_out(2),
  read_thread()
{
  find_endpoints();
  if (true) // FIXME
  {
    std::cout << "EP(IN):  " << endpoint_in << std::endl;
    std::cout << "EP(OUT): " << endpoint_out << std::endl;
  }

  int ret = libusb_open(dev, &handle);

  if (0)
  {
    int err;
    if ((err = libusb_set_configuration(handle, 0)) < 0)
    {
      std::ostringstream out;
      out << "Error set USB configuration: " << usb_strerror(err) << std::endl
          << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
      throw std::runtime_error(out.str());
    }
  }

  if (ret != LIBUSB_SUCCESS)
  {
    throw std::runtime_error("Error opening Xbox360 controller");
  }
  else
  {
    // FIXME: bInterfaceNumber shouldn't be hardcoded
    int err = usb_claim_n_detach_interface(handle, 0, try_detach);
    if (err != 0) 
    {
      std::ostringstream out;
      out << " Error couldn't claim the USB interface: " << usb_strerror(err) << std::endl
          << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
      throw std::runtime_error(out.str());
    }
  }

#ifdef LIBUSB_OLD_VERSION
  if (0)
  {
    uint8_t arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 255 };
    for (int len = 3; len <= 8; ++len)
    {
      // Sending random data:
      for (int front = 0; front < 256; ++front)
      {
        for (size_t i = 0; i < sizeof(arr); ++i)
        {
          uint8_t ledcmd[] = { front, len, arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i] }; 
          printf("%d %d %d\n", len, front, arr[i]);
          int transferred = 0;
          
          int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                              ledcmd, len, &transferred, 0);

          if (ret != LIBUSB_SUCCESS)
          {
            throw std::runtime_error("--- failure ---"); // FIXME
          }

          uint8_t data[32];
          int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_IN | endpoint_in, 
                                              reinterpret_cast<char*>(data), sizeof(data), 20);
          print_raw_data(std::cout, data, ret);
        }
      }
    }
  }
#endif

  read_thread.reset(new USBReadThread(handle, endpoint_in, 32));
  read_thread->start_thread();

  if (chatpad)
  {
#ifdef LIBUSB_OLD_VERSION
    m_chatpad.reset(new Chatpad(handle, dev->descriptor.bcdDevice, chatpad_no_init, chatpad_debug));
    m_chatpad->send_init();
    m_chatpad->start_threads();
#endif
  }

  if (headset)
  {
    m_headset.reset(new Headset(handle, headset_debug, headset_dump, headset_play));
  }
}

Xbox360Controller::~Xbox360Controller()
{
  read_thread->stop_thread();

  if (m_chatpad.get())
  {
    m_chatpad.reset();
  }

  if (m_headset.get())
  {
    m_headset.reset();
  }

  libusb_release_interface(handle, 0); 
  libusb_close(handle);
}

void
Xbox360Controller::find_endpoints()
{
  libusb_config_descriptor* config;
  int ret = libusb_get_config_descriptor(dev, 0 /* config_index */, &config);
  if (ret != LIBUSB_SUCCESS)
  {
    throw std::runtime_error("-- failure --"); // FIXME
  }

  bool debug_print = true;

  for(const libusb_interface* interface = config->interface;
      interface != config->interface + config->bNumInterfaces;
      ++interface)
  {
    for(const libusb_interface_descriptor* altsetting = interface->altsetting;
        altsetting != interface->altsetting + interface->num_altsetting;
        ++altsetting)
    {
      if (debug_print) std::cout << "  Interface: " << static_cast<int>(altsetting->bInterfaceNumber) << std::endl;
          
      for(const libusb_endpoint_descriptor* endpoint = altsetting->endpoint; 
          endpoint != altsetting->endpoint + altsetting->bNumEndpoints; 
          ++endpoint)
      {
        if (debug_print) 
          std::cout << "    Endpoint: " << int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK)
                    << "(" << ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? "IN" : "OUT") << ")"
                    << std::endl;

        if (altsetting->bInterfaceClass    == LIBUSB_CLASS_VENDOR_SPEC &&
            altsetting->bInterfaceSubClass == 93 &&
            altsetting->bInterfaceProtocol == 1)
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
Xbox360Controller::set_rumble(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  int transferred = 0;
  int ret = 0;
  ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                  rumblecmd, sizeof(rumblecmd), 
                                  &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    throw std::runtime_error("-- failure --"); // FIXME
  }
}

void
Xbox360Controller::set_led(uint8_t status)
{
  uint8_t ledcmd[] = { 0x01, 0x03, status }; 
  int transferred = 0;
  int ret = 0;
  ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                  ledcmd, sizeof(ledcmd), 
                                  &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    throw std::runtime_error("-- failure --"); // FIXME
  }
}

bool
Xbox360Controller::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  uint8_t data[32];
  int ret = 0;
  int len = 0;

  if (read_thread.get())
  {
    ret = read_thread->read(data, sizeof(data), &len, timeout);
  }
  else
  {
    ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_IN | endpoint_in, 
                                    data, sizeof(data), 
                                    &len, timeout);
  }

  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  { // Error
    std::ostringstream str;
    str << "Xbox360Controller: USBError: " << ret << "\n" << usb_strerror(ret);
    throw std::runtime_error(str.str());
  }
  else if (len == 0)
  {
    if (verbose)
    {
      std::cout << "zero length read" << std::endl;
      // happens with the Xbox360 controller every now and then, just
      // ignore, seems harmless, so just ignore
    }
  }
  else if (len == 3 && data[0] == 0x01 && data[1] == 0x03)
  { 
    if (verbose)
    {
      std::cout << "Xbox360Controller: LED Status: " << int(data[2]) << std::endl;
    }
  }
  else if (len == 3 && data[0] == 0x03 && data[1] == 0x03)
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
  else if (len == 3 && data[0] == 0x08 && data[1] == 0x03)
  {
    if (!g_options->quiet)
    {
      if (data[2] == 0x00)
        std::cout << "Headset: none";
      else if (data[2] == 0x02)
        std::cout << "Headset: none";
    }
  }
  else if (len == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg.type    = XBOX_MSG_XBOX360;
    memcpy(&msg.xbox360, data, sizeof(Xbox360Msg));
    return true;
  }
  else
  {
    std::cout << "Unknown: ";
    print_raw_data(std::cout, data, len);
  }

  return false;
}

/* EOF */
