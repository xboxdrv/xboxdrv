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

#include "controller/generic_usb_controller.hpp"

#include <iostream>

#include "util/string.hpp"
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
    print(config, std::cout);

    if (config->bNumInterfaces <= m_interface)
    {
      raise_exception(std::runtime_error, "interface " << m_interface << " not available");
    }

    if (config->interface[m_interface].num_altsetting == 0)
    {
      raise_exception(std::runtime_error, "no interface descriptors available");
    }

    // search for the given endpoint
    const libusb_endpoint_descriptor* ep = 0;
    for(int i = 0; i < config->interface[m_interface].altsetting[0].bNumEndpoints; ++i)
    {
      ep = &(config->interface[m_interface].altsetting[0].endpoint[i]);

      if ((ep->bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK) == m_endpoint)
      {
        break;
      }
      else
      {
        ep = 0;
      }
    }

    if (!ep)
    {
      raise_exception(std::runtime_error, "endpoint " << m_endpoint << " not available");
    }
    else
    {
      uint16_t wMaxPacketSize = ep->wMaxPacketSize;

      log_debug("wMaxPacketSize: {}", wMaxPacketSize);
      usb_claim_interface(m_interface, try_detach);
      usb_submit_read(m_endpoint, wMaxPacketSize);
    }
  }
}

GenericUSBController::~GenericUSBController()
{
}

void
GenericUSBController::print(libusb_config_descriptor* cfg, std::ostream& out) const
{
  for(int i = 0; i < cfg->bNumInterfaces; ++i)
  {
    std::cout << "Interface " << i << std::endl;
    for(int j = 0; j < cfg->interface[i].num_altsetting; ++j)
    {
      std::cout << "  AltSetting " << j << std::endl;
      for(int k = 0; k < cfg->interface[i].altsetting[j].bNumEndpoints; ++k)
      {
        const libusb_endpoint_descriptor& ep = cfg->interface[i].altsetting[j].endpoint[k];
        std::cout << "    Endpoint " << k << " "
                  << static_cast<int>(ep.bEndpointAddress & LIBUSB_ENDPOINT_ADDRESS_MASK)
                  << ((ep.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) ? " IN" : " OUT")
                  << std::endl;
      }
    }
  }
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
GenericUSBController::parse(const uint8_t* data, int len, ControllerMessage* msg_out)
{
  std::cout << "GenericUSBController:parse(): " << raw2str(data, len) << std::endl;
  return false;
}

/* EOF */
