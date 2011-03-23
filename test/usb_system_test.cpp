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

#include <libusb.h>
#include <glib.h>
#include <stdexcept>

#include "log.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"
#include "usb_gsource.hpp"
#include "dummy_message_processor.hpp"
#include "xbox360_controller.hpp"
#include "xboxmsg.hpp"

libusb_device* get_controller_dev()
{
  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;
    
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      log_warn("libusb_get_device_descriptor() failed: " << usb_strerror(ret));
    }
    else
    {
      if (desc.idVendor == 0x045e && desc.idProduct == 0x028e)
      {
        libusb_ref_device(dev);
        libusb_free_device_list(list, 1 /* unref_devices */);
        return dev;
      }
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);

  return 0;
}

void process_msg(const XboxGenericMsg& msg)
{
  log_debug(msg);
}

int main()
{
  g_logger.set_log_level(Logger::kDebug);

  //g_type_init();

  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_init() failed: " << usb_strerror(ret));
  }

  GMainLoop* m_gmain = g_main_loop_new(NULL, FALSE);

  USBGSource usb_gsource;
  usb_gsource.attach(NULL);

  libusb_device* dev = get_controller_dev();
  assert(dev);
  Xbox360Controller* controller = new Xbox360Controller(dev,
                                                        false, false, false,
                                                        false, 
                                                        false,
                                                        "",
                                                        "",
                                                        false);
  controller->set_led(2);
  g_main_loop_run(m_gmain);
  
  return 0;
}

/* EOF */
