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


std::ostream& operator<<(std::ostream& out, const struct ff_envelope& envelope)
{
  out << "attack_length: " << envelope.attack_length
      << " attack_level: " << envelope.attack_level
      << " fade_length: " << envelope.fade_length
      << " fade_level: " << envelope.fade_level;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_replay& replay)
{
  out << "length: " << replay.length << " delay: " << replay.delay;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_trigger& trigger)
{
  out << "button: " << trigger.button << " interval: " << trigger.interval;
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct ff_effect& effect)
{
  switch (effect.type)
    {
      case FF_CONSTANT:
        out << "FF_CONSTANT "
            << "level: " << effect.u.constant.level
            << " envelope: { " << effect.u.constant.envelope << " }";
        break;

      case FF_PERIODIC:
        out << "FF_PERIODIC"
            << " waveform: " << effect.u.periodic.waveform
            << " period: " << effect.u.periodic.period
            << " magnitude: " << effect.u.periodic.magnitude
            << " offset: " << effect.u.periodic.offset
            << " phase: " << effect.u.periodic.phase
            << " envelope: { " << effect.u.periodic.envelope << " }";
        break;

      case FF_RAMP:
        out << "FF_RAMP " 
            << "start_level: " << effect.u.ramp.start_level
            << "end_level: " << effect.u.ramp.end_level
            << "envelope: { " <<  effect.u.ramp.envelope << " }";
        break;

      case FF_SPRING:
        out << "FF_SPRING";
        break;

      case FF_FRICTION:
        out << "FF_FRICTION";
        break;

      case FF_DAMPER:
        out << "FF_DAMPER";
        break;

      case FF_RUMBLE:
        out << "FF_RUMBLE: "
            << "strong_magnitude: " << effect.u.rumble.strong_magnitude
            << " weak_magnitude: " << effect.u.rumble.weak_magnitude;
        break;

      case FF_INERTIA:
        out << "FF_INERTIA";
        break;

      case FF_CUSTOM:
        out << "FF_CUSTOM";
        break;

      default:
        out << "FF_<unknown>";
        break;
    }

  out << "\n";
  out << "direction: " << effect.direction << "\n";
  out << "replay: " << effect.replay << "\n";
  out << "trigger: " << effect.trigger << "\n";

  return out;
}

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
  std::fill_n(ff_lst,  FF_CNT,  false);

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
LinuxUinput::add_ff(uint16_t code)
{
  if (!ff_lst[code])
    {
      ff_lst[code] = true;

      if (!ff_bit)
        {
          ioctl(fd, UI_SET_EVBIT, EV_FF);
          ff_bit = true;
        }

      ioctl(fd, UI_SET_FFBIT, code);
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

  if (ff_bit)
    user_dev.ff_effects_max = 64;

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

void
LinuxUinput::update(float delta)
{
  if (ff_bit)
    {
      struct input_event ev;

      int ret = read(fd, &ev, sizeof(ev));
      if (ret < 0)
        {
          if (errno != EAGAIN)
            std::cout << "Error: " << strerror(errno) << " " << ret << std::endl;
        }
      else if (ret == sizeof(ev))
        { // successful read
          std::cout << "type: " << ev.type << " code: " << ev.code << " value: " << ev.value << std::endl;

          switch(ev.type)
            {
              case EV_LED:
                if (ev.code == LED_MISC)
                  {
                    // FIXME: implement this
                    std::cout << "unimplemented: Set LED status: " << ev.value << std::endl;
                  }
                break;

              case EV_FF:
                std::cout << "EV_FF: playing effect: effect_id = " << ev.code << " value: " << ev.value << std::endl;
                break;

              case EV_UINPUT:
                switch (ev.code)
                  {
                    case UI_FF_UPLOAD:
                      {
                        struct uinput_ff_upload upload;
                        memset(&upload, 0, sizeof(upload));

                        // *VERY* important, without this you
                        // break the kernel and have to reboot due
                        // to dead hanging process
                        upload.request_id = ev.value;

                        ioctl(fd, UI_BEGIN_FF_UPLOAD, &upload);

                        std::cout << "XXX FF_UPLOAD: rumble upload:"
                                  << " effect_id: " << upload.effect.id
                                  << " effect_type: " << upload.effect.type
                                  << std::endl;
                        std::cout << "EFFECT: " << upload.effect << std::endl;

                        upload.retval = 0;
                            
                        ioctl(fd, UI_END_FF_UPLOAD, &upload);
                      }
                      break;

                    case UI_FF_ERASE:
                      {
                        struct uinput_ff_erase erase;
                        memset(&erase, 0, sizeof(erase));

                        // *VERY* important, without this you
                        // break the kernel and have to reboot due
                        // to dead hanging process
                        erase.request_id = ev.value;

                        ioctl(fd, UI_BEGIN_FF_ERASE, &erase);

                        std::cout << "FF_ERASE: rumble erase: effect_id = " << erase.effect_id << std::endl;
                        erase.retval = 0; // FIXME: is this used?
                            
                        ioctl(fd, UI_END_FF_ERASE, &erase);
                      }
                      break;

                    default: 
                      std::cout << "Unhandled event code read" << std::endl;
                      break;
                  }
                break;

              default:
                std::cout << "Unhandled event type read: " << ev.type << std::endl;
                break;
            }
          std::cout << "--------------------------------" << std::endl;
        }
      else
        {
          std::cout << "uInput::update: short read: " << ret << std::endl;
        }
    }
}


/* EOF */
