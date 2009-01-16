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
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "evdev_helper.hpp"
#include "linux_uinput.hpp"

LinuxUinput::LinuxUinput(const std::string& name, uint16_t vendor, uint16_t product)
  : name(name),
    vendor(vendor),
    product(product),
    fd(-1),
    key_bit(false),
    rel_bit(false),
    abs_bit(false),
    led_bit(false),
    ff_bit(false)
{
  std::fill_n(abs_lst, ABS_CNT, false);
  std::fill_n(rel_lst, REL_CNT, false);
  std::fill_n(key_lst, KEY_CNT, false);

  memset(&user_dev, 0, sizeof(uinput_user_dev));

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
  // std::cout << "add_abs: " << abs2str(code) << " (" << min << ", " << max << ") " << name << std::endl;

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
  // std::cout << "add_rel: " << rel2str(code) << " " << name << std::endl;

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
  // std::cout << "add_key: " << btn2str(code) << " " << name << std::endl;

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
LinuxUinput::finish()
{
  strncpy(user_dev.name, name.c_str(), UINPUT_MAX_NAME_SIZE);
  user_dev.id.version = 0;
  user_dev.id.bustype = BUS_USB;
  user_dev.id.vendor  = vendor;
  user_dev.id.product = product;

  //std::cout << "Finalizing uinput: '" << user_dev.name << "'" << std::endl;

  if (write(fd, &user_dev, sizeof(user_dev)) < 0)
    throw std::runtime_error("uinput:finish: " + name + strerror(errno));

  if (ioctl(fd, UI_DEV_CREATE))
    {
      std::ostringstream out;
      out << "LinuxUinput: Unable to create UINPUT device: '" << name << "': " << strerror(errno);
      throw std::runtime_error(out.str());
    }
}

void
LinuxUinput::send(uint16_t type, uint16_t code, int32_t value)
{
  struct input_event ev;      
  memset(&ev, 0, sizeof(ev));

  gettimeofday(&ev.time, NULL);
  ev.type  = type;
  ev.code  = code;
  if (ev.type == EV_KEY)
    ev.value = (value>0) ? 1 : 0;
  else
    ev.value = value;

  if (write(fd, &ev, sizeof(ev)) < 0)
    throw std::runtime_error(std::string("uinput:send_button: ") + strerror(errno)); 
}

/* EOF */
