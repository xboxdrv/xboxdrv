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

#include "generic_usb_controller.hpp"

#include <iostream>

#include "helper.hpp"
#include "raise_exception.hpp"

GenericUSBController::GenericUSBController(libusb_device* dev, 
                                           int interface, int endpoint,
                                           bool try_detach) :
  USBController(dev),
  m_interface(interface),
  m_endpoint(endpoint)
{
  struct libusb_config_descriptor* config;
  if (libusb_get_active_config_descriptor(dev, &config) != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "failed to get config descriptor");
  }
  else
  {
    if (config->bNumInterfaces == 0)
    {
      raise_exception(std::runtime_error, "no interfaces available");
    }
    
    if (config->interface[0].num_altsetting == 0)
    {
      raise_exception(std::runtime_error, "no interface descriptors available");
    }

    if (config->interface[0].altsetting[0].bNumEndpoints <= m_endpoint)
    {
      raise_exception(std::runtime_error, "endpoint not available");
    }

    uint16_t wMaxPacketSize = config->interface[0].altsetting[0].endpoint[m_endpoint].wMaxPacketSize;

    log_debug("wMaxPacketSize: " << wMaxPacketSize);
    usb_claim_interface(m_interface, try_detach);
    usb_submit_read(m_endpoint, wMaxPacketSize);
  }
}

GenericUSBController::~GenericUSBController()
{
  usb_cancel_read();
  usb_release_interface(m_interface);
}

void
GenericUSBController::set_rumble_real(uint8_t left, uint8_t right)
{
  std::cout << "GenericUSBController::set_rumble(" << static_cast<int>(left) << ", " << static_cast<int>(right) << ")" << std::endl;
}

void
GenericUSBController::set_led_real(uint8_t status)
{
  std::cout << "GenericUSBController::set_led(" << static_cast<int>(status) << ")" << std::endl;
}

bool
GenericUSBController::parse(uint8_t* data, int len, ControllerMessage* msg_out)
{
  std::cout << "GenericUSBController:parse(): " << raw2str(data, len) << std::endl;
  return false;
}

/* EOF */
