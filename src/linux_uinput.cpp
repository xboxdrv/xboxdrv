/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "linux_uinput.hpp"

LinuxUinput::LinuxUinput()
  : key_bit(false),
    rel_bit(false),
    abs_bit(false),
    led_bit(false),
    ff_bit(false)
{
  std::fill_n(abs_lst, ABS_CNT, false);
  std::fill_n(rel_lst, REL_CNT, false);
  std::fill_n(key_lst, KEY_CNT, false);

  // Open the input device
  const char* uinput_filename[] = { "/dev/input/uinput", "/dev/uinput", "/dev/misc/uinput" };
  const int uinput_filename_count = (sizeof(uinput_filename)/sizeof(char*));

  for (int i = 0; i < uinput_filename_count; ++i) 
    {
      if ((fd = open(uinput_filename[i], O_RDWR | O_NDELAY)) >= 0)
        {
          break;
        }
      else
        {
          std::cout << "Error: " << uinput_filename[i] << ": " << strerror(errno) << std::endl;
        }
    }

  if (fd < 0)
    {
      std::cout << "Error: No stuitable uinput device found" << std::endl;
      std::cout << "" << std::endl;
      std::cout << "Troubleshooting:" << std::endl;
      std::cout << "  * make sure uinput kernel module is loaded " << std::endl;
      std::cout << "  * make sure joydev kernel module is loaded " << std::endl;
      std::cout << "  * make sure you have permissions to access the uinput device" << std::endl;
      std::cout << "  * start the driver with ./xboxdrv -v --no-uinput to see if the driver itself works" << std::endl;
      std::cout << "" << std::endl;
      exit(EXIT_FAILURE);
    }
}

LinuxUinput::~LinuxUinput()
{
  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
}

void
LinuxUinput::add_abs(uint16_t code, int min, int max)
{
  if (!abs_lst[code])
    {
      abs_lst[code] = true;

      if (!abs_bit)
        {
          ioctl(fd, UI_SET_EVBIT, EV_ABS);
          abs_bit = true;
        }

      ioctl(fd, UI_SET_ABSBIT, code);

      user_dev.absmin[code] = min;
      user_dev.absmax[code] = max; 
    }
}

void
LinuxUinput::add_rel(uint16_t code)
{
  if (!rel_lst[code])
    {
      rel_lst[code] = true;

      if (!rel_bit)
        {
          ioctl(fd, UI_SET_EVBIT, EV_REL);
          rel_bit = true;
        }

      ioctl(fd, UI_SET_RELBIT, code);
    }
}

void
LinuxUinput::add_key(uint16_t code)
{
  if (!key_lst[code])
    {
      key_lst[code] = true;

      if (!key_bit)
        {
          ioctl(fd, UI_SET_EVBIT, EV_KEY);
          key_bit = true;
        }

      ioctl(fd, UI_SET_KEYBIT, code);
    }
}

void
LinuxUinput::finish(const char* name)
{
  std::cout << "Finalizing UInput" << std::endl;
  strncpy(user_dev.name, name, UINPUT_MAX_NAME_SIZE);
  user_dev.id.version = 0;
  user_dev.id.bustype = BUS_USB;
  user_dev.id.vendor  = 0x045e; // FIXME: this shouldn't be hardcoded
  user_dev.id.product = 0x028e;

  if (write(fd, &user_dev, sizeof(user_dev)) < 0)
    throw std::runtime_error(std::string("uinput:finish: ") + strerror(errno));

  if (ioctl(fd, UI_DEV_CREATE))
    {
      std::cout << "Unable to create UINPUT device." << std::endl;
    }
}

void
LinuxUinput::send_button(uint16_t code, int32_t value)
{
  struct input_event ev;      
  memset(&ev, 0, sizeof(ev));

  gettimeofday(&ev.time, NULL);
  ev.type  = EV_KEY;
  ev.code  = code;
  ev.value = (value>0) ? 1 : 0;

  if (write(fd, &ev, sizeof(ev)) < 0)
    throw std::runtime_error(std::string("uinput:send_button: ") + strerror(errno)); 
}

void
LinuxUinput::send_axis(uint16_t code, int32_t value)
{
  struct input_event ev;      
  memset(&ev, 0, sizeof(ev));

  gettimeofday(&ev.time, NULL);
  ev.type  = EV_ABS;
  ev.code  = code;
  ev.value = value;

  if (write(fd, &ev, sizeof(ev)) < 0)
    throw std::runtime_error(std::string("uinput:send_axis: ") + strerror(errno));
}

/* EOF */
