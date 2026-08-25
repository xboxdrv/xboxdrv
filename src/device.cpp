// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "device.hpp"

#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include <fmt/format.h>
#include <logmich/log.hpp>

#include "force_feedback_handler.hpp"

namespace uinpp {

Device::Device(DeviceType device_type, std::string const& name,
               const struct input_id& iid) :
  m_device_type(device_type),
  m_iid(iid),
  m_name(name),
  m_finished(false),
  m_fd(-1),
  m_key_bit(false),
  m_rel_bit(false),
  m_abs_bit(false),
  m_led_bit(false),
  m_ff_bit(false),
  m_ff_handler(nullptr),
  m_ff_callback(),
  m_needs_sync(true)
{
  log_debug("{} {}:{}", m_name, iid.vendor, iid.product);

  std::fill_n(m_abs_lst, ABS_CNT, false);
  std::fill_n(m_rel_lst, REL_CNT, false);
  std::fill_n(m_key_lst, KEY_CNT, false);
  std::fill_n(m_ff_lst,  FF_CNT,  false);

  // Open the input device
  char const* uinput_filename[] = { "/dev/input/uinput", "/dev/uinput", "/dev/misc/uinput" };
  const int uinput_filename_count = static_cast<int>(sizeof(uinput_filename)/sizeof(char const*));

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

Device::~Device()
{
  ioctl(m_fd, UI_DEV_DESTROY);
  close(m_fd);
}

void
Device::set_phys(std::string_view phys)
{
  std::string phys_str(phys);
  if (ioctl(m_fd, UI_SET_PHYS, phys_str.c_str()) < 0) {
    throw std::runtime_error(fmt::format("Device::set_phys() failed: ", strerror(errno)));
  }
}

void
Device::set_prop(int value)
{
  if (ioctl(m_fd, UI_SET_PROPBIT, value) < 0) {
    throw std::runtime_error(fmt::format("Device::set_prop() failed: ", strerror(errno)));
  }
}

void
Device::add_abs(uint16_t code, int min, int max, int fuzz, int flat, int resolution)
{
  log_debug("add_abs: {} ({}, {})", code, min, max);

  if (!m_abs_lst[code])
  {
    m_abs_lst[code] = true;

    if (!m_abs_bit)
    {
      ioctl(m_fd, UI_SET_EVBIT, EV_ABS);
      m_abs_bit = true;
    }

    uinput_abs_setup abs_setup;
    memset(&abs_setup, 0, sizeof(abs_setup));

    abs_setup.code = code;
    abs_setup.absinfo.minimum = min;
    abs_setup.absinfo.maximum = max;
    abs_setup.absinfo.fuzz = fuzz;
    abs_setup.absinfo.flat = flat;
    abs_setup.absinfo.resolution = resolution;

    if (ioctl(m_fd, UI_ABS_SETUP, &abs_setup) < 0) {
      throw std::runtime_error(fmt::format("UI_ABS_SETUP failed: {}", strerror(errno)));
    }
  }
}

void
Device::add_rel(uint16_t code)
{
  log_debug("add_rel: {} {}", code, m_name);

  if (!m_rel_lst[code])
  {
    m_rel_lst[code] = true;

    if (!m_rel_bit)
    {
      ioctl(m_fd, UI_SET_EVBIT, EV_REL);
      m_rel_bit = true;
    }

    ioctl(m_fd, UI_SET_RELBIT, code);
  }
}

void
Device::add_key(uint16_t code)
{
  log_debug("add_key: {} {}", code, m_name);

  if (!m_key_lst[code])
  {
    m_key_lst[code] = true;

    if (!m_key_bit)
    {
      ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
      m_key_bit = true;
    }

    ioctl(m_fd, UI_SET_KEYBIT, code);
  }
}

void
Device::add_ff(uint16_t code)
{
  if (!m_ff_lst[code])
  {
    m_ff_lst[code] = true;

    if (!m_ff_bit)
    {
      ioctl(m_fd, UI_SET_EVBIT, EV_FF);
      m_ff_bit = true;
      assert(m_ff_handler == nullptr);
      m_ff_handler = new ForceFeedbackHandler();
    }

    ioctl(m_fd, UI_SET_FFBIT, code);
  }
}

void
Device::set_ff_callback(const std::function<void (uint8_t, uint8_t)>& callback)
{
  m_ff_callback = callback;
}

void
Device::finish()
{
  assert(!m_finished);

  // Create some mandatory events that are needed for the kernel/Xorg
  // to register the device as its proper type
  switch(m_device_type)
  {
    case DeviceType::GENERIC:
      // nothing to be done
      break;

    case DeviceType::KEYBOARD:
      // FIXME: do something clever
      break;

    case DeviceType::MOUSE:
      add_rel(REL_X);
      add_rel(REL_Y);
      add_key(BTN_LEFT);
      break;

    case DeviceType::JOYSTICK:
      // the kernel and SDL have different rules for joystick
      // detection, so this is more a hack then a proper solution
      if (!m_key_lst[BTN_A])
      {
        add_key(BTN_A);
      }

      if (!m_abs_lst[ABS_X])
      {
        add_abs(ABS_X, -1, 1, 0, 0);
      }

      if (!m_abs_lst[ABS_Y])
      {
        add_abs(ABS_Y, -1, 1, 0, 0);
      }
      break;
  }

  {
    uinput_setup setup;
    memset(&setup, 0, sizeof(setup));

    setup.id = m_iid;

    strncpy(setup.name, m_name.c_str(), UINPUT_MAX_NAME_SIZE - 1);
    setup.name[UINPUT_MAX_NAME_SIZE - 1] = '\0';

    if (m_ff_bit) {
      setup.ff_effects_max = m_ff_handler->get_max_effects();
    } else {
      setup.ff_effects_max = 0;
    }

    log_debug("'{}' {}:{}", setup.name, setup.id.vendor, setup.id.product);

    if (ioctl(m_fd, UI_DEV_SETUP, &setup) < 0) {
      throw std::runtime_error(fmt::format("UI_DEV_SETUP failed: {}", strerror(errno)));
    }
  }

  // FIXME: check that the config isn't empty and give a more
  // meaningful message when it is

  log_debug("finish");
  if (ioctl(m_fd, UI_DEV_CREATE))
  {
    throw std::runtime_error(fmt::format("unable to create uinput device: '{}': ", m_name, strerror(errno)));
  }

  m_finished = true;
}

void
Device::send(uint16_t type, uint16_t code, int32_t value)
{
  m_needs_sync = true;

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
Device::sync()
{
  if (m_needs_sync)
  {
    send(EV_SYN, SYN_REPORT, 0);
    m_needs_sync = false;
  }
}

void
Device::update(int msec_delta)
{
  if (m_ff_bit)
  {
    assert(m_ff_handler);

    m_ff_handler->update(msec_delta);

    log_info("{:5d} {:5d}", m_ff_handler->get_strong_magnitude(), m_ff_handler->get_weak_magnitude());

    if (m_ff_callback)
    {
      m_ff_callback(static_cast<unsigned char>(m_ff_handler->get_strong_magnitude() / 128),
                    static_cast<unsigned char>(m_ff_handler->get_weak_magnitude()   / 128));
    }
  }
}

void
Device::read()
{
  struct input_event ev;
  ssize_t ret;

  while((ret = ::read(m_fd, &ev, sizeof(ev))) == sizeof(ev))
  {
    switch(ev.type)
    {
      case EV_LED:
        if (ev.code == LED_MISC)
        {
          // FIXME: implement this
          log_info("unimplemented: set LED status: {}", ev.value);
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
        log_warn("unhandled event type read: {}", ev.type);
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
      log_error("failed to read from file description: {}: {}", ret, strerror(errno));
    }
  }
  else
  {
    log_error("short read: {}", ret);
  }
}

} // namespace uinpp

/* EOF */
