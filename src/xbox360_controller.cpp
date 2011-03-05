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

#include "xbox360_controller.hpp"

#include "chatpad.hpp"
#include "headset.hpp"
#include "helper.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"

Xbox360Controller::Xbox360Controller(libusb_device* dev,
                                     bool chatpad, bool chatpad_no_init, bool chatpad_debug, 
                                     bool headset, 
                                     bool headset_debug, 
                                     const std::string& headset_dump,
                                     const std::string& headset_play,
                                     bool try_detach) :
  USBController(dev),
  dev_type(),
  endpoint_in(1),
  endpoint_out(2),
  m_chatpad(),
  m_headset()
{
  // find endpoints
  find_endpoints(dev);
  log_debug("EP(IN):  " << endpoint_in);
  log_debug("EP(OUT): " << endpoint_out);

  // claim interface
  claim_interface(0, try_detach);

  // create chatpad
  if (chatpad)
  {
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << usb_strerror(ret));    }
    else
    {
      m_chatpad.reset(new Chatpad(m_handle, desc.bcdDevice, chatpad_no_init, chatpad_debug));
      m_chatpad->send_init();
      m_chatpad->start_threads();
    }
  }

  // create headset
  if (headset)
  {
    m_headset.reset(new Headset(m_handle, headset_debug, headset_dump, headset_play));
  }
}

Xbox360Controller::~Xbox360Controller()
{
  release_interface(0);
}

void
Xbox360Controller::find_endpoints(libusb_device* dev)
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
      log_debug("Interface: " << static_cast<int>(altsetting->bInterfaceNumber));
          
      for(const libusb_endpoint_descriptor* endpoint = altsetting->endpoint; 
          endpoint != altsetting->endpoint + altsetting->bNumEndpoints; 
          ++endpoint)
      {
        log_debug("    Endpoint: " << int(endpoint->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK) <<
                  "(" << ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? "IN" : "OUT") << ")");

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
  ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                  rumblecmd, sizeof(rumblecmd), 
                                  &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

void
Xbox360Controller::set_led(uint8_t status)
{
  uint8_t ledcmd[] = { 0x01, 0x03, status }; 
  int transferred = 0;
  int ret = 0;
  ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_OUT | endpoint_out, 
                                  ledcmd, sizeof(ledcmd), 
                                  &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

bool
Xbox360Controller::read(XboxGenericMsg& msg, int timeout)
{
  uint8_t data[32];
  int len = 0;

  int ret = libusb_interrupt_transfer(m_handle, LIBUSB_ENDPOINT_IN | endpoint_in, 
                                      data, sizeof(data), 
                                      &len, timeout);

  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else if (ret != LIBUSB_SUCCESS)
  { // Error
    raise_exception(std::runtime_error, "libusb_interrupt_transfer(): " << usb_strerror(ret));
  }
  else if (len == 0)
  {
    // happens with the Xbox360 controller every now and then, just
    // ignore, seems harmless, so just ignore
    log_debug("zero length read");
  }
  else if (len == 3 && data[0] == 0x01 && data[1] == 0x03)
  { 
    log_debug("Xbox360Controller: LED Status: " << int(data[2]));
  }
  else if (len == 3 && data[0] == 0x03 && data[1] == 0x03)
  { 
    // data[2] == 0x00 means that rumble is disabled
    // data[2] == 0x01 unknown, but rumble works
    // data[2] == 0x02 unknown, but rumble works
    // data[2] == 0x03 is default with rumble enabled
    log_info("rumble status: " << int(data[2]));
  }
  else if (len == 3 && data[0] == 0x08 && data[1] == 0x03)
  {
    // FIXME: maybe a proper indicator for the actvity on the chatpad
    // port, so that we don't have to send chatpad init
    if (data[2] == 0x00)
    {
      log_info("peripheral: none");
    }
    else if (data[2] == 0x01)
    {
      log_info("peripheral: chatpad");
    }
    else if (data[2] == 0x02)
    {
      log_info("peripheral: headset");
    }
    else if (data[2] == 0x03)
    {
      log_info("peripheral: headset, chatpad");
    }
    else
    {
      log_info("peripheral: unknown: " << int(data[2]));
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
    log_debug("unknown: " << raw2str(data, len));
  }

  return false;
}

/* EOF */
