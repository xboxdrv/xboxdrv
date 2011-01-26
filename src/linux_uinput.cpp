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

#include "linux_uinput.hpp"

#include <boost/format.hpp>
#include <errno.h>
#include <fcntl.h>

#include "evdev_helper.hpp"
#include "force_feedback_handler.hpp"
#include "log.hpp"
#include "raise_exception.hpp"

LinuxUinput::LinuxUinput(DeviceType device_type, const std::string& name_, uint16_t vendor_, uint16_t product_) :
  m_device_type(device_type),
  name(name_),
  vendor(vendor_),
  product(product_),
  m_finished(false),
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
  log_debug(name << " " << vendor << ":" << product);

  std::fill_n(abs_lst, ABS_CNT, false);
  std::fill_n(rel_lst, REL_CNT, false);
  std::fill_n(key_lst, KEY_CNT, false);
  std::fill_n(ff_lst,  FF_CNT,  false);

  memset(&user_dev, 0, sizeof(uinput_user_dev));

  // Open the input device
  const char* uinput_filename[] = { "/dev/input/uinput", "/dev/uinput", "/dev/misc/uinput" };
  const int uinput_filename_count = (sizeof(uinput_filename)/sizeof(const char*));

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
    std::ostringstream out;
    out << "\nError: No stuitable uinput device found, tried:" << std::endl;
    out << std::endl;
    out << str.str();
    out << "" << std::endl;
    out << "Troubleshooting:" << std::endl;
    out << "  * make sure uinput kernel module is loaded " << std::endl;
    out << "  * make sure joydev kernel module is loaded " << std::endl;
    out << "  * make sure you have permissions to access the uinput device" << std::endl;
    out << "  * start the driver with ./xboxdrv -v --no-uinput to see if the driver itself works" << std::endl;
    out << "" << std::endl;

    throw std::runtime_error(out.str());
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
  log_debug("add_abs: " << abs2str(code) << " (" << min << ", " << max << ") " << name);

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
  log_debug("add_rel: " << rel2str(code) << " " << name);

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
  log_debug("add_key: " << key2str(code) << " " << name);

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
  assert(!m_finished);

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

  log_debug("'" << user_dev.name << "' " << user_dev.id.vendor << ":" << user_dev.id.product);

  if (ff_bit)
    user_dev.ff_effects_max = ff_handler->get_max_effects();

  //std::cout << "Finalizing uinput: '" << user_dev.name << "'" << std::endl;

  {
    int write_ret = write(fd, &user_dev, sizeof(user_dev));
    if (write_ret < 0)
    {
      throw std::runtime_error("uinput:finish: " + name + ": " + strerror(errno));
    }
    else
    {
      log_debug("write return value: " << write_ret);
    }
  }

  // FIXME: check that the config isn't empty and give a more
  // meaningful message when it is

  log_debug("finish");
  if (ioctl(fd, UI_DEV_CREATE))
  {
    raise_exception(std::runtime_error, "unable to create uinput device: '" << name << "': " << strerror(errno));
  }

  m_finished = true;
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
LinuxUinput::update(int msec_delta)
{
  if (ff_bit)
  {
    assert(ff_handler);

    ff_handler->update(msec_delta);

    log_info(boost::format("%5d %5d") % ff_handler->get_strong_magnitude() % ff_handler->get_weak_magnitude());

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
      {
        log_error("failed to read from file description: " << ret << ": " << strerror(errno));
      }
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
            log_info("unimplemented: set LED status: " << ev.value);
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
              log_warn("unhandled event code read");
              break;
          }
          break;

        default:
          log_warn("unhandled event type read: " << ev.type);
          break;
      }
    }
    else
    {
      log_warn("short read: " << ret);
    }
  }
}


/* EOF */
