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
#include "raise_exception.hpp"

LinuxUinput::LinuxUinput(DeviceType device_type, const std::string& name_, 
                         const struct input_id& usbid_) :
  m_device_type(device_type),
  name(name_),
  usbid(usbid_),
  m_finished(false),
  m_fd(-1),
  m_io_channel(),
  m_source_id(),
  user_dev(),
  key_bit(false),
  rel_bit(false),
  abs_bit(false),
  led_bit(false),
  ff_bit(false),
  m_ff_handler(0),
  m_ff_callback(),
  needs_sync(true)
{
  log_debug(name << " " << usbid.vendor << ":" << usbid.product);

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
    if ((m_fd = open(uinput_filename[i], O_RDWR | O_NDELAY)) >= 0)
    {
      break;
    }
    else
    {
      str << "  " << uinput_filename[i] << ": " << strerror(errno) << std::endl;
    }
  }

  if (m_fd < 0)
  {
    std::ostringstream out;
    out << "\nError: No suitable uinput device found, tried:" << std::endl;
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
  g_source_remove(m_source_id);

  ioctl(m_fd, UI_DEV_DESTROY);
  close(m_fd);
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
      ioctl(m_fd, UI_SET_EVBIT, EV_ABS);
      abs_bit = true;
    }

    ioctl(m_fd, UI_SET_ABSBIT, code);

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
      ioctl(m_fd, UI_SET_EVBIT, EV_REL);
      rel_bit = true;
    }

    ioctl(m_fd, UI_SET_RELBIT, code);
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
      ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
      key_bit = true;
    }

    ioctl(m_fd, UI_SET_KEYBIT, code);
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
      ioctl(m_fd, UI_SET_EVBIT, EV_FF);
      ff_bit = true;
      assert(m_ff_handler == 0);
      m_ff_handler = new ForceFeedbackHandler();
    }

    ioctl(m_fd, UI_SET_FFBIT, code);
  }  
}

void
LinuxUinput::set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback)
{
  m_ff_callback = callback;
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

    case kKeyboardDevice:
      // FIXME: do something clever
      break;

    case kMouseDevice:
      add_rel(REL_X);
      add_rel(REL_Y);
      add_key(BTN_LEFT);
      break;

    case kJoystickDevice:
      // the kernel and SDL have different rules for joystick
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
  user_dev.id.version = usbid.version;
  user_dev.id.bustype = usbid.bustype;
  user_dev.id.vendor  = usbid.vendor;
  user_dev.id.product = usbid.product;

  log_debug("'" << user_dev.name << "' " << user_dev.id.vendor << ":" << user_dev.id.product);

  if (ff_bit)
  {
    user_dev.ff_effects_max = m_ff_handler->get_max_effects();
  }

  {
    int write_ret = write(m_fd, &user_dev, sizeof(user_dev));
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
  if (ioctl(m_fd, UI_DEV_CREATE))
  {
    raise_exception(std::runtime_error, "unable to create uinput device: '" << name << "': " << strerror(errno));
  }

  m_finished = true;

  {
    // start g_io_channel
    m_io_channel = g_io_channel_unix_new(m_fd);

    // set encoding to binary
    GError* error = NULL;
    if (g_io_channel_set_encoding(m_io_channel, NULL, &error) != G_IO_STATUS_NORMAL)
    {
      log_error(error->message);
      g_error_free(error);
    }

    g_io_channel_set_buffered(m_io_channel, false);

    m_source_id = g_io_add_watch(m_io_channel, 
                                 static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                                 &LinuxUinput::on_read_data_wrap, this);
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

  if (write(m_fd, &ev, sizeof(ev)) < 0)
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
    assert(m_ff_handler);

    m_ff_handler->update(msec_delta);

    log_info(boost::format("%5d %5d") % m_ff_handler->get_strong_magnitude() % m_ff_handler->get_weak_magnitude());

    if (m_ff_callback)
    {
      m_ff_callback(static_cast<unsigned char>(m_ff_handler->get_strong_magnitude() / 128),
                    static_cast<unsigned char>(m_ff_handler->get_weak_magnitude()   / 128));
    }
  }
}

gboolean
LinuxUinput::on_read_data(GIOChannel* source, GIOCondition condition)
{
  struct input_event ev;
  int ret;

  while((ret = read(m_fd, &ev, sizeof(ev))) == sizeof(ev))
  {
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
            m_ff_handler->set_gain(ev.value);
            break;

          default:
            if (ev.value)
              m_ff_handler->play(ev.code);
            else
              m_ff_handler->stop(ev.code);
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

              ioctl(m_fd, UI_BEGIN_FF_UPLOAD, &upload);
              m_ff_handler->upload(upload.effect);
              upload.retval = 0;
                            
              ioctl(m_fd, UI_END_FF_UPLOAD, &upload);
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

              ioctl(m_fd, UI_BEGIN_FF_ERASE, &erase);
              m_ff_handler->erase(erase.effect_id);
              erase.retval = 0;
                            
              ioctl(m_fd, UI_END_FF_ERASE, &erase);
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

  if (ret == 0)
  {
    // ok, no more data
  }
  else if (ret < 0)
  {
    if (errno != EAGAIN)
    {
      log_error("failed to read from file description: " << ret << ": " << strerror(errno));
    }
  }
  else
  {
    log_error("short read: " << ret);
  }
  
  return TRUE;
}


/* EOF */
