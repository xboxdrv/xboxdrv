/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#ifndef HEADER_XBOX360_USB_THREAD_HPP
#define HEADER_XBOX360_USB_THREAD_HPP

#include <inttypes.h>
#include <pthread.h>
#include <queue>
#include "xboxdrv.hpp"

/** A worker thread that reads from the USB device in a blocking
    fashion, so that the Xbox360Driver can stay non blocking. 
    
    Using the timeout value of usb_interrupt_read() should in theory
    allow that too, but it didn't work for some reason, events got
    lost, thus this seperate thread. A libusb alternative/fork called
    openusb might fix this propblem by providing asynchronous io.
*/
class Xbox360UsbThread
{
private:
  struct usb_device*     dev;
  struct usb_dev_handle* handle;
  bool thread_quit;
  std::queue<Xbox360Msg> mailbox;

public:
  Xbox360UsbThread(struct usb_device* dev);
  ~Xbox360UsbThread();
  
  void start();
  void stop();

  void set_led(uint8_t led_status);
  void set_rumble(uint8_t big, uint8_t small);

  bool has_msg() const;
  Xbox360Msg pop_msg();

private:
  static void* thread_loop_wrap(void* userdata);
  void* thread_loop();

  Xbox360UsbThread (const Xbox360UsbThread&);
  Xbox360UsbThread& operator= (const Xbox360UsbThread&);
};

#endif

/* EOF */
