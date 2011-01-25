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

#include "xbox360_wireless_controller.hpp"

#include <sstream>
#include <boost/format.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"
#include "xboxmsg.hpp"

Xbox360WirelessController::Xbox360WirelessController(libusb_device* dev_, int controller_id, 
                                                     bool try_detach) :
  dev(dev_),
  handle(),
  endpoint(),
  interface(),
  battery_status(),
  serial(),
  led_status(0)
{
  assert(controller_id >= 0 && controller_id < 4);
  
  // FIXME: Is hardcoding those ok?
  endpoint  = controller_id*2 + 1;
  interface = controller_id*2;

  const int ret = libusb_open(dev, &handle);
  if (ret != LIBUSB_SUCCESS)
  {
    throw std::runtime_error("Xbox360WirelessController: Error opening Xbox360 controller");
  }
  else
  {
    int err = usb_claim_n_detach_interface(handle, interface, try_detach);
    if (err != 0) 
    {
      std::ostringstream out;
      out << "Error couldn't claim the USB interface: " << usb_strerror(err) << std::endl
          << "Try to run 'rmmod xpad' and then xboxdrv again or start xboxdrv with the option --detach-kernel-driver.";
      throw std::runtime_error(out.str());
    }
  }
}

Xbox360WirelessController::~Xbox360WirelessController()
{
  libusb_release_interface(handle, interface); 
  libusb_close(handle);
}

void
Xbox360WirelessController::set_rumble(uint8_t left, uint8_t right)
{
  //                                       +-- typo? might be 0x0c, i.e. length
  //                                       v
  uint8_t rumblecmd[] = { 0x00, 0x01, 0x0f, 0xc0, 0x00, left, right, 0x00, 0x00, 0x00, 0x00, 0x00 };
  int transferred = 0;
  int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint,
                                      rumblecmd, sizeof(rumblecmd), &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

void
Xbox360WirelessController::set_led(uint8_t status)
{
  led_status = status;
  //                                +--- Why not just status?
  //                                v
  uint8_t ledcmd[] = { 0x00, 0x00, 0x08, 0x40 + (status % 0x0e), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  int transferred = 0;
  int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_OUT | endpoint, 
                                ledcmd, sizeof(ledcmd), &transferred, 0);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_interrupt_transfer() failed: " << usb_strerror(ret));
  }
}

bool
Xbox360WirelessController::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  uint8_t data[32];
  int len = 0;
 
  int ret = libusb_interrupt_transfer(handle, LIBUSB_ENDPOINT_IN | endpoint, 
                                      data, sizeof(data), 
                                      &len, timeout);

  if (ret == LIBUSB_ERROR_TIMEOUT)
  {
    return false;
  }
  else  if (ret != LIBUSB_SUCCESS)
  { // Error
    std::ostringstream str;
    str << "USBError: " << ret << "\n" << usb_strerror(ret);
    throw std::runtime_error(str.str());
  }
  else if (len == 2 && data[0] == 0x08) 
  { // Connection Status Message
    if (data[1] == 0x00) 
    {
      std::cout << "Connection status: nothing" << std::endl;
    } 
    else if (data[1] == 0x80) 
    {
      std::cout << "Connection status: controller connected" << std::endl;
      set_led(led_status);
    } 
    else if (data[1] == 0x40) 
    {
      std::cout << "Connection status: headset connected" << std::endl;
    }
    else if (data[1] == 0xc0) 
    {
      std::cout << "Connection status: controller and headset connected" << std::endl;
      set_led(led_status);
    }
    else
    {
      std::cout << "Connection status: unknown" << std::endl;
    }
  }
  else if (len == 29)
  {
    if (data[0] == 0x00 && data[1] == 0x0f && data[2] == 0x00 && data[3] == 0xf0)
    { // Initial Announc Message
      serial = (boost::format("%2x:%2x:%2x:%2x:%2x:%2x:%2x")
                % int(data[7])
                % int(data[8])
                % int(data[9])
                % int(data[10])
                % int(data[11])
                % int(data[12])
                % int(data[13])).str();
      battery_status = data[17];
      std::cout << "Serial: " << serial << std::endl;
      std::cout << "Battery Status: " << battery_status << std::endl;
    }
    else if (data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0xf0 && data[4] == 0x00 && data[5] == 0x13)
    { // Event message
      msg.type    = XBOX_MSG_XBOX360;
      memcpy(&msg.xbox360, data+4, sizeof(Xbox360Msg));
      return true;
    }
    else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x13)
    { // Battery status
      battery_status = data[4];
      std::cout << "Battery Status: " << battery_status << std::endl;
    }
    else if (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0xf0)
    {
      // 0x00 0x00 0x00 0xf0 0x00 ... is send after each button
      // press, doesn't seem to contain any information
    }
    else
    {
      std::cout << "Unknown: ";
      print_raw_data(std::cout, data, len);
    }
  }
  else if (len == 0)
  {
    // ignore
  }
  else
  {
    std::cout << "Unknown: ";
    print_raw_data(std::cout, data, len);
  }

  return false;
}

/* EOF */
