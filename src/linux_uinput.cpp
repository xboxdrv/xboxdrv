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

#include <boost/format.hpp>
#include <assert.h>
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
#include "force_feedback_handler.hpp"
#include "linux_uinput.hpp"

LinuxUinput::LinuxUinput(DeviceType device_type, const std::string& name_, uint16_t vendor_, uint16_t product_) :
  m_device_type(device_type),
  name(name_),
  vendor(vendor_),
  product(product_),
  fd(-1),
  user_dev(),
  key_bit(false),
  rel_bit(false),
  abs_bit(false),
  led_bit(false),
  ff_bit(false),
  ff_handler(0),
  ff_callback(),
  needs_sync(true)
{
  std::fill_n(abs_lst, ABS_CNT, false);
  std::fill_n(rel_lst, REL_CNT, false);
  std::fill_n(key_lst, KEY_CNT, false);
  std::fill_n(ff_lst,  FF_CNT,  false);

  memset(&user_dev, 0, sizeof(uinput_user_dev));

  // Open the input device
  const char* uinput_filename[] = { "/dev/input/uinput", "/dev/uinput", "/dev/misc/uinput" };
  const int uinput_filename_count = (sizeof(uinput_filename)/sizeof(char*));

  std::ostringstream str;
  for (int i = 0; i < uinput_filename_count; ++i)
  {
    if ((fd = open(uinput_filename[i], O_RDWR | O_NDELAY)) >= 0)
    {
      break;
    }
    else
    {
      str << "  " << uinput_filename[i] << ": " << strerror(errno) << std::endl;
    }
  }

  if (fd < 0)
  {
    std::cout << "\nError: No stuitable uinput device found, tried:" << std::endl;
    std::cout << std::endl;
    std::cout << str.str();
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
LinuxUinput::add_abs(uint16_t code, int min, int max, int fuzz, int flat)
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
    user_dev.absfuzz[code] = fuzz;
    user_dev.absflat[code] = flat;
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
      assert(ff_handler == 0);
      ff_handler = new ForceFeedbackHandler();
    }

    ioctl(fd, UI_SET_FFBIT, code);
  }  
}

void
LinuxUinput::set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback)
{
  ff_callback = callback;
}

void
LinuxUinput::finish()
{
  // Create some mandatory events that are needed for the kernel/Xorg
  // to register the device as its proper type
  switch(m_device_type)
  {
    case kGenericDevice:
      // nothing to be done
      break;

    case kMouseDevice:
      add_rel(REL_X);
      add_rel(REL_Y);
      add_key(BTN_LEFT);
      break;

    case kJoystickDevice:
      // FIXME: the kernel and SDL have different rules for joystick
      // detection, so this is more a hack then a proper solution
      if (!key_lst[BTN_A])
      {
        add_key(BTN_A);
      }

      if (!abs_lst[ABS_X])
      {
        add_abs(ABS_X, -1, 1, 0, 0);
      }

      if (!abs_lst[ABS_Y])
      {
        add_abs(ABS_Y, -1, 1, 0, 0);
      }
      break;
  }

  strncpy(user_dev.name, name.c_str(), UINPUT_MAX_NAME_SIZE);
  user_dev.id.version = 0x114; // FIXME: whats this for?
  user_dev.id.bustype = BUS_USB;
  user_dev.id.vendor  = vendor;
  user_dev.id.product = product;

  if (ff_bit)
    user_dev.ff_effects_max = ff_handler->get_max_effects();

  //std::cout << "Finalizing uinput: '" << user_dev.name << "'" << std::endl;

  if (write(fd, &user_dev, sizeof(user_dev)) < 0)
    throw std::runtime_error("uinput:finish: " + name + ": " + strerror(errno));

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
  needs_sync = true;

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
LinuxUinput::sync()
{
  if (needs_sync)
  {
    send(EV_SYN, SYN_REPORT, 0);
    needs_sync = false;
  }
}

void
LinuxUinput::update_force_feedback(int msec_delta)
{
  if (ff_bit)
  {
    assert(ff_handler);

    ff_handler->update(msec_delta);

    if (0)
      std::cout << boost::format("%5d %5d") 
        % ff_handler->get_weak_magnitude() 
        % ff_handler->get_strong_magnitude() << std::endl;

    if (ff_callback)
    {
      ff_callback(static_cast<unsigned char>(ff_handler->get_strong_magnitude() / 128),
                  static_cast<unsigned char>(ff_handler->get_weak_magnitude()   / 128));
                      
    }
      
    struct input_event ev;

    int ret = read(fd, &ev, sizeof(ev));
    if (ret < 0)
    {
      if (errno != EAGAIN)
        std::cout << "Error: " << strerror(errno) << " " << ret << std::endl;
    }
    else if (ret == sizeof(ev))
    { // successful read
      //std::cout << "type: " << ev.type << " code: " << ev.code << " value: " << ev.value << std::endl;

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
          switch(ev.code)
          {
            case FF_GAIN:
              ff_handler->set_gain(ev.value);
              break;

            default:
              if (ev.value)
                ff_handler->play(ev.code);
              else
                ff_handler->stop(ev.code);
          }
          break;

        case EV_UINPUT:
          switch (ev.code)
          {
            case UI_FF_UPLOAD:
            {
              struct uinput_ff_upload upload;
              memset(&upload, 0, sizeof(upload));

              // *VERY* important, without this you break
              // the kernel and have to reboot due to dead
              // hanging process
              upload.request_id = ev.value;

              ioctl(fd, UI_BEGIN_FF_UPLOAD, &upload);
              ff_handler->upload(upload.effect);
              upload.retval = 0;
                            
              ioctl(fd, UI_END_FF_UPLOAD, &upload);
            }
            break;

            case UI_FF_ERASE:
            {
              struct uinput_ff_erase erase;
              memset(&erase, 0, sizeof(erase));

              // *VERY* important, without this you break
              // the kernel and have to reboot due to dead
              // hanging process
              erase.request_id = ev.value;

              ioctl(fd, UI_BEGIN_FF_ERASE, &erase);
              ff_handler->erase(erase.effect_id);
              erase.retval = 0;
                            
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
    }
    else
    {
      std::cout << "uInput::update: short read: " << ret << std::endl;
    }
  }
}


/* EOF */
