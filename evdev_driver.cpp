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


#include <assert.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <errno.h>

#include "evdev_driver.hpp"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

EvdevDriver::EvdevDriver(const std::string& filename)
{
  memset(abs2idx, 0, sizeof(abs2idx));
  memset(rel2idx, 0, sizeof(rel2idx));
  memset(key2idx, 0, sizeof(key2idx));

  fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);

  if (fd == -1)
    {
      throw std::runtime_error(filename + ": " + std::string(strerror(errno)));
    }

  if (ioctl(fd, EVIOCGVERSION, &version)) 
    {
      throw std::runtime_error("Error: EvdevDevice: Couldn't get version for " + filename);
    }

    { // FIXME: Some versions of linux don't have these structs, use arrays there
      struct input_id id;
      ioctl(fd, EVIOCGID, &id);
      printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
             id.bustype, id.vendor, id.product, id.vendor);
    }

  { // Get the human readable name
    char c_name[256] = "unknown";
    ioctl(fd, EVIOCGNAME(sizeof(c_name)), c_name);
    name = c_name;
    std::cout << "Name: " << name << std::endl;
  }

  { // Read in how many btn/abs/rel the device has
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
    memset(bit, 0, sizeof(bit));
    ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);

    unsigned long abs_bit[NBITS(ABS_MAX)];
    unsigned long rel_bit[NBITS(REL_MAX)];
    unsigned long key_bit[NBITS(KEY_MAX)];

    memset(abs_bit, 0, sizeof(abs_bit));
    memset(rel_bit, 0, sizeof(rel_bit));
    memset(key_bit, 0, sizeof(key_bit));

    ioctl(fd, EVIOCGBIT(EV_ABS, ABS_MAX), abs_bit);
    ioctl(fd, EVIOCGBIT(EV_REL, REL_MAX), rel_bit);
    ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), key_bit);

    for(int i = 0; i < ABS_MAX; ++i)
      {
        if (test_bit(i, abs_bit))
          {
            struct input_absinfo absinfo;
            ioctl(fd, EVIOCGABS(i), &absinfo);
            std::cout << "Abs: " << i << " min: " << absinfo.minimum << " max: " << absinfo.maximum << std::endl;
            abs2idx[i] = abs_port_out.size();
            abs_port_out.push_back(new AbsPortOut("EvdevDriver:abs", absinfo.minimum, absinfo.maximum));
          }
      }

    for(int i = 0; i < REL_MAX; ++i)
      {
        if (test_bit(i, rel_bit))
          {
            std::cout << "Rel: " << i << std::endl;
            rel2idx[i] = rel_port_out.size();
            rel_port_out.push_back(new RelPortOut("EvdevDriver:rel"));
          }
      }

    for(int i = 0; i < KEY_MAX; ++i)
      {
        if (test_bit(i, key_bit))
          {
            key2idx[i] = btn_port_out.size();
            btn_port_out.push_back(new BtnPortOut("EvdevDriver:btn"));
          }
      }
  }
}

void
EvdevDriver::update(float delta)
{
  struct input_event ev[128];
  // FIXME: turn this into a while loop so all events get processed
  int rd = read(fd, ev, sizeof(struct input_event) * 128);
  //std::cout << rd / sizeof(struct input_event) << std::endl;
  if (rd >= int(sizeof(struct input_event)))
    {
      for (int i = 0; i < rd / (int)sizeof(struct input_event); ++i)
        {
          //std::cout << ev[i].type << " " << ev[i].code << " " << ev[i].value << std::endl;

          switch (ev[i].type)
            {
              case EV_ABS:
                abs_port_out[abs2idx[ev[i].code]]->set_state(ev[i].value);
                break;

              case EV_REL:
                rel_port_out[rel2idx[ev[i].code]]->set_state(ev[i].value);
                break;

              case EV_KEY:
                btn_port_out[key2idx[ev[i].code]]->set_state(ev[i].value);
                break;

              default:
                // ignore everything else
                break;
            }
        }
    }
}

/* EOF */
