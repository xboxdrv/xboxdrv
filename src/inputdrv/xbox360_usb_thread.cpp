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

#include <usb.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/format.hpp>
#include "log.hpp"
#include "../xboxmsg.hpp"
#include "xbox360_usb_thread.hpp"

Xbox360UsbThread::Xbox360UsbThread(struct usb_device* dev)
  : thread_quit(false)
{
  handle = usb_open(dev); // FIXME: add some error checking
  if (usb_claim_interface(handle, 0) != 0) // FIXME: bInterfaceNumber shouldn't be hardcoded
    std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
}

Xbox360UsbThread::~Xbox360UsbThread()
{
  usb_close(handle);
}

void
Xbox360UsbThread::start()
{
  pthread_t thread;
  if (pthread_create(&thread, NULL, Xbox360UsbThread::thread_loop_wrap, this) != 0)
    {
      LOG("Fatal Error creating the Xbox360UsbThread thread");
    }
  else
    {
      LOG("launch_thread_loop");
    }
}

void
Xbox360UsbThread::stop()
{ // FIXME: This alone isn't enough, since usb_interrupt_read blocks
  thread_quit = true;
}

void
Xbox360UsbThread::set_led(uint8_t led_status)
{ // Should be ok without mutex lock
  char ledcmd[] = { 1, 3, led_status }; 
  usb_interrupt_write(handle, 2, ledcmd, 3, 0);
}

void
Xbox360UsbThread::set_rumble(uint8_t big, uint8_t small)
{
  // Should be ok without mutex lock
  char rumblecmd[] = { 0x00, 0x08, 0x00, big, small, 0x00, 0x00, 0x00 };
  usb_interrupt_write(handle, 2, rumblecmd, 8, 0);
}

void*
Xbox360UsbThread::thread_loop()
{
  thread_quit = false;
  while (!thread_quit)
    {
      uint8_t data[20];
      int ret = usb_interrupt_read(handle, 1 /*EndPoint*/, (char*)data, sizeof(data), 0 /*Timeout*/);
      if (ret < 0)
        { // Error
          std::ostringstream str;
          str << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
          str << "Shutting down" << std::endl;
          throw std::runtime_error(str.str());
        }
      else if (ret == 0) // ignore
        {
          // happen with the Xbox360 every now and then, just
          // ignore, seems harmless
        }
      else if (ret == 3) // ignore
        {
          // This data gets send when the controller is accessed the
          // first time after being connected to the USB bus, no idea
          // what it means, it seems to be identical for different
          // controllers, we just ignore it
          //
          // len: 3 Data: 0x01 0x03 0x0e
          // len: 3 Data: 0x02 0x03 0x00
          // len: 3 Data: 0x03 0x03 0x03
          // len: 3 Data: 0x08 0x03 0x00
          // len: 3 Data: 0x01 0x03 0x00
        }
      else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
        {
          //LOG("Data in Mailbox");
          Xbox360Msg& msg = (Xbox360Msg&)data;
          // mutex this
          mailbox.push(msg);
        }
      else
        {
          std::cout << "Unknown data: bytes: " << ret << " Data: ";
          for(int j = 0; j < ret; ++j)
            std::cout << boost::format("0x%02x ") % int(data[j]);
          std::cout << std::endl;     
        }
    }

  return NULL;      
}

void*
Xbox360UsbThread::thread_loop_wrap(void* userdata)
{
  Xbox360UsbThread* drv = static_cast<Xbox360UsbThread*>(userdata);
  return drv->thread_loop();
}

bool
Xbox360UsbThread::has_msg() const
{
  // FIXME: mutex lock this
  return !mailbox.empty();
}

Xbox360Msg
Xbox360UsbThread::pop_msg()
{
  // FIXME: mutex lock this
  Xbox360Msg msg = mailbox.front();
  mailbox.pop();
  return msg;
}

/* EOF */
