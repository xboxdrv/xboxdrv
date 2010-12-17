/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "evdev_controller.hpp"

#include <boost/format.hpp>
#include <linux/input.h>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "evdev_helper.hpp"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

EvdevController::EvdevController(const std::string& filename,
                                 const EvdevAbsMap& absmap,
                                 const std::map<int, XboxButton>& keymap) :
  m_fd(-1),
  m_name(),
  m_absmap(absmap),
  m_keymap(keymap),
  m_absinfo(ABS_MAX),
  m_event_buffer(),
  m_msg()
{
  memset(&m_msg, 0, sizeof(m_msg));
  m_msg.type = XBOX_MSG_XBOX360;

  m_fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);

  if (m_fd == -1)
  {
    throw std::runtime_error(filename + ": " + std::string(strerror(errno)));
  }

  { // Get the human readable name
    char c_name[1024] = "unknown";
    ioctl(m_fd, EVIOCGNAME(sizeof(c_name)), c_name);
    m_name = c_name;
    std::cout << "Name: " << m_name << std::endl;
  }

  { // grab the device, so it doesn't broadcast events into the wild
    int ret = ioctl(m_fd, EVIOCGRAB, 1);
    if ( ret == -1 )
    {
      close(m_fd);
      throw std::runtime_error(strerror(errno));
    }
  }

  { // Read in how many btn/abs/rel the device has
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
    memset(bit, 0, sizeof(bit));
    ioctl(m_fd, EVIOCGBIT(0, EV_MAX), bit[0]);

    unsigned long abs_bit[NBITS(ABS_MAX)];
    unsigned long rel_bit[NBITS(REL_MAX)];
    unsigned long key_bit[NBITS(KEY_MAX)];

    memset(abs_bit, 0, sizeof(abs_bit));
    memset(rel_bit, 0, sizeof(rel_bit));
    memset(key_bit, 0, sizeof(key_bit));

    ioctl(m_fd, EVIOCGBIT(EV_ABS, ABS_MAX), abs_bit);
    ioctl(m_fd, EVIOCGBIT(EV_REL, REL_MAX), rel_bit);
    ioctl(m_fd, EVIOCGBIT(EV_KEY, KEY_MAX), key_bit);

    for(int i = 0; i < ABS_MAX; ++i)
    {
      if (test_bit(i, abs_bit))
      {
        struct input_absinfo absinfo;
        ioctl(m_fd, EVIOCGABS(i), &absinfo);
        
        std::cout << boost::format("abs: %-20s min: %6d max: %6d") % abs2str(i) % absinfo.minimum % absinfo.maximum << std::endl;
        m_absinfo[i] = absinfo;
        //abs2idx[i] = abs_port_out.size();
        //abs_port_out.push_back(new AbsPortOut("EvdevDriver:abs", absinfo.minimum, absinfo.maximum));
      }
    }

    for(int i = 0; i < REL_MAX; ++i)
    {
      if (test_bit(i, rel_bit))
      {
        std::cout << "rel: " << rel2str(i) << std::endl;
        //rel2idx[i] = rel_port_out.size();
        //rel_port_out.push_back(new RelPortOut("EvdevDriver:rel"));
      }
    }

    for(int i = 0; i < KEY_MAX; ++i)
    {
      if (test_bit(i, key_bit))
      {
        std::cout << "key: " << key2str(i) << std::endl;
        //key2idx[i] = btn_port_out.size();
        //btn_port_out.push_back(new BtnPortOut("EvdevDriver:btn"));
      }
    }
  }
}

void
EvdevController::set_rumble(uint8_t left, uint8_t right)
{
  // not implemented
}

void
EvdevController::set_led(uint8_t status)
{
  // not implemented
}

bool
EvdevController::apply(XboxGenericMsg& msg, const struct input_event& ev)
{
  //std::cout << ev.type << " " << ev.code << " " << ev.value << std::endl;
  switch(ev.type)
  {
    case EV_KEY:
      {
        KeyMap::iterator it = m_keymap.find(ev.code);
        if (it != m_keymap.end())
        {
          set_button(msg, it->second, ev.value);
          return true;
        }
        else
        {
          return false;
        }
      }
      break;

    case EV_ABS:
      {
        const struct input_absinfo& absinfo = m_absinfo[ev.code];
        m_absmap.process(msg, ev.code, ev.value, absinfo.minimum, absinfo.maximum);
        return true; // FIXME: wrong
        break;
      }

    default:
      // not supported event
      return false;
      break;
  }
}

void
EvdevController::read_data_to_buffer()
{
  struct input_event ev[128];
  int rd = 0;
  while((rd = ::read(m_fd, ev, sizeof(struct input_event) * 128)) > 0)
  {
    for (size_t i = 0; i < rd / sizeof(struct input_event); ++i)
    {
      m_event_buffer.push(ev[i]);
    }
  }
}

bool
EvdevController::read(XboxGenericMsg& msg, bool verbose, int timeout)
{
  read_data_to_buffer();

  while(!m_event_buffer.empty())
  {
    struct input_event ev = m_event_buffer.front();
    m_event_buffer.pop();

    if (ev.type == EV_SYN)
    {
      msg = m_msg;
      return true;
    }
    else
    {
      apply(m_msg, ev);
    }
  }

  usleep(timeout * 1000);

  return false;
}

/* EOF */
