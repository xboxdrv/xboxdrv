/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include "xboxdrv_main.hpp"

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <libusb.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <format>
#include <functional>

#include <uinpp/multi_device.hpp>
#include <uinpp/linux.hpp>
#include <unsebu/usb_gsource.hpp>
#include <unsebu/usb_helper.hpp>
#include <unsebu/usb_subsystem.hpp>

#include "controller/evdev_controller.hpp"
#include "controller/wiimote_controller.hpp"
#include "controller_factory.hpp"
#include "controller_slot_config.hpp"
#include "controller_thread.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "util/exec.hpp"
#include "util/string.hpp"

namespace xboxdrv {

using namespace std::placeholders;

XboxdrvMain* XboxdrvMain::s_current = nullptr;

namespace {

bool find_controller_by_path(std::string const& busid_str, std::string const& devid_str,
                        libusb_device** xbox_device)
{
  int busid = str2int(busid_str);
  int devid = str2int(devid_str);

  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];

    if (libusb_get_bus_number(dev)     == busid &&
        libusb_get_device_address(dev) == devid)
    {
      *xbox_device = dev;

      // incrementing ref count, user must call unref
      libusb_ref_device(*xbox_device);
      libusb_free_device_list(list, 1 /* unref_devices */);
      return true;
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

bool find_controller_by_id(int id, int vendor_id, int product_id, libusb_device** xbox_device)
{
  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  int id_count = 0;
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      log_warn("libusb_get_device_descriptor() failed: {}", libusb_strerror(ret));
    }
    else
    {
      if (desc.idVendor  == vendor_id &&
          desc.idProduct == product_id)
      {
        if (id_count == id)
        {
          *xbox_device = dev;
          // increment ref count, user must free the device
          libusb_ref_device(*xbox_device);
          libusb_free_device_list(list, 1 /* unref_devices */);
          return true;
        }
        else
        {
          id_count += 1;
        }
      }
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

bool find_xbox360_controller(int id, libusb_device** xbox_device, XPadDevice* type)
{
  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  int id_count = 0;
  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      log_warn("libusb_get_device_descriptor() failed: {}", libusb_strerror(ret));
    }
    else
    {
      for(int i = 0; i < xpad_devices_count; ++i)
      {
        if (desc.idVendor  == xpad_devices[i].idVendor &&
            desc.idProduct == xpad_devices[i].idProduct)
        {
          if (id_count == id)
          {
            *xbox_device = dev;
            *type        = xpad_devices[i];
            // increment ref count, user must free the device
            libusb_ref_device(*xbox_device);
            libusb_free_device_list(list, 1 /* unref_devices */);
            return true;
          }
          else
          {
            id_count += 1;
          }
        }
      }
    }
  }

  libusb_free_device_list(list, 1 /* unref_devices */);
  return false;
}

void find_controller(libusb_device** dev, XPadDevice& dev_type, Options const& opts)
{
  if (opts.busid[0] != '\0' && opts.devid[0] != '\0')
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      throw std::runtime_error("--device-by-path BUS:DEV option must be used in combination with --type TYPE option");
    }
    else
    {
      if (!find_controller_by_path(opts.busid, opts.devid, dev))
      {
        raise_exception(std::runtime_error, "couldn't find device " << opts.busid << ":" << opts.devid);
      }
      else
      {
        dev_type.type      = opts.gamepad_type;
        dev_type.name      = "unknown";
        libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(*dev, &desc) == LIBUSB_SUCCESS)
        {
          dev_type.idVendor  = desc.idVendor;
          dev_type.idProduct = desc.idProduct;
        }
      }
    }
  }
  else if (opts.vendor_id != -1 && opts.product_id != -1)
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      throw std::runtime_error("--device-by-id VENDOR:PRODUCT option must be used in combination with --type TYPE option");
    }
    else
    {
      if (!find_controller_by_id(opts.controller_id, opts.vendor_id, opts.product_id, dev))
      {
        raise_exception(std::runtime_error, "couldn't find device with "
                        << std::format("{:04x}:{:04x}", opts.vendor_id, opts.product_id));
      }
      else
      {
        dev_type.type = opts.gamepad_type;
        dev_type.idVendor  = static_cast<uint16_t>(opts.vendor_id);
        dev_type.idProduct = static_cast<uint16_t>(opts.product_id);
        dev_type.name = "unknown";
      }
    }
  }
  else
  {
    if (!find_xbox360_controller(opts.controller_id, dev, &dev_type))
    {
      throw std::runtime_error("No Xbox or Xbox360 controller found");
    }
  }
}

} // namespace

XboxdrvMain::XboxdrvMain(unsebu::USBSubsystem& usb_subsystem, Options const& opts) :
  m_usb_subsystem(usb_subsystem),
  m_opts(opts),
  m_gmain(),
  m_usb_gsource(),
  m_uinput(),
  m_use_libusb(false),
  m_dev_type(),
  m_controller()
{
  assert(!s_current);
  s_current = this;

  m_gmain = g_main_loop_new(NULL, false);

  signal(SIGINT,  &XboxdrvMain::on_sigint);
  signal(SIGTERM, &XboxdrvMain::on_sigint);
}

XboxdrvMain::~XboxdrvMain()
{
  signal(SIGINT,  NULL);
  signal(SIGTERM, NULL);

  s_current = nullptr;

  g_main_loop_unref(m_gmain);
}

ControllerPtr
XboxdrvMain::create_controller()
{
  if (!m_opts.evdev_device.empty())
  { // normal PC joystick via evdev
    return ControllerPtr(new EvdevController(m_opts.evdev_device,
                                             m_opts.evdev_absmap,
                                             m_opts.evdev_keymap,
                                             m_opts.evdev_relmap,
                                             m_opts.evdev_grab,
                                             m_opts.evdev_debug));

    // FIXME: ugly, should be part of Controller
    m_dev_type.type = GAMEPAD_XBOX360;
    m_dev_type.idVendor  = 0;
    m_dev_type.idProduct = 0;
    m_dev_type.name = "Evdev device";
  }
  else if (m_opts.wiimote)
  {
#ifdef HAVE_CWIID
    log_debug("Creating Wiimote controller");
    return ControllerPtr(new WiimoteController);
#else
    throw std::runtime_error("libcwiid not found at compile time, Wiimote support is not available");
#endif
  }
  else
  { // regular USB Xbox360-like controller

    // USBController refs the device; unrefs in its destructor
    libusb_device* dev = nullptr;

    find_controller(&dev, m_dev_type, m_opts);

    if (!dev)
    {
      throw std::runtime_error("no suitable USB device found, abort");
    }
    else
    {
      if (!m_opts.quiet)
      {
        print_info(dev, m_dev_type, m_opts);
      }

      return ControllerFactory::create(m_dev_type, dev, m_opts);
    }
  }
}

void
XboxdrvMain::init_controller(ControllerPtr const& controller)
{
  // Player LED follows controller slot (same as daemon mode), not a guessed
  // /dev/input/jsN index — that race caused wrong rings (see issue #168).
  if (m_opts.get_controller_slot().get_led_status() == -1)
  {
    controller->set_led(2); // slot 0 → player 1 (LED pattern 2)
  }
  else
  {
    controller->set_led(static_cast<uint8_t>(m_opts.get_controller_slot().get_led_status()));
  }

  if (m_opts.rumble_l != -1 && m_opts.rumble_r != -1)
  { // Only set rumble when explicitly requested
    controller->set_rumble(static_cast<uint8_t>(m_opts.rumble_l),
                           static_cast<uint8_t>(m_opts.rumble_r));
  }
}

void
XboxdrvMain::on_controller_disconnect()
{
  shutdown();
}

void
XboxdrvMain::run()
{
  m_controller = create_controller();
  m_controller->set_disconnect_cb(std::bind(&XboxdrvMain::on_controller_disconnect, this));
  init_controller(m_controller);

  if (m_opts.instant_exit)
  {
    usleep(1000);
  }
  else
  {
    ControllerSlotConfigPtr config_set;
    if (m_opts.no_uinput)
    {
      if (!m_opts.quiet)
      {
        std::cout << "Starting without uinput" << std::endl;
      }
    }
    else
    {
      log_debug("creating UInput");
      m_uinput = std::make_unique<uinpp::MultiDevice>();
      m_uinput->set_extra_events(m_opts.extra_events);
      m_uinput->set_device_names(m_opts.uinput_device_names);
      m_uinput->set_device_usbids(m_opts.uinput_device_usbids);

      log_debug("creating ControllerSlotConfig");
      config_set = ControllerSlotConfig::create(*m_uinput,
                                                0, m_opts.extra_devices,
                                                m_opts.get_controller_slot());

      // After all the ControllerConfig registered their events, finish up
      // the device creation
      log_debug("finish UInput creation");
      m_uinput->finish();
    }

    if (!m_opts.quiet)
    {
      print_feature_status();
      print_device_nodes();

      if (m_opts.silent)
      {
        std::cout << "\nPress Ctrl-C to quit" << std::endl;
      }
      else
      {
        std::cout << "\nPress Ctrl-C to quit, use '--silent' to suppress the event output" << std::endl;
      }
    }

    {
      ControllerThread thread(m_controller, config_set, m_opts);
      log_debug("launching thread");

      pid_t pid = 0;
      if (!m_opts.exec.empty())
      {
        pid = spawn_exe(m_opts.exec);
        g_child_watch_add(pid, &XboxdrvMain::on_child_watch_wrap, this);
      }

      log_debug("launching main loop");
      g_main_loop_run(m_gmain);

      m_controller.reset();
    }

    if (!m_opts.quiet)
    {
      std::cout << "Shutdown complete" << std::endl;
    }
  }
}

void
XboxdrvMain::on_child_watch(GPid pid, gint status)
{
  log_info("child processes exited with status: {}", status);
  shutdown();
}

void
XboxdrvMain::print_info(libusb_device* dev, XPadDevice const& dev_type, Options const& opts) const
{
  libusb_device_descriptor desc;
  int ret = libusb_get_device_descriptor(dev, &desc);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_get_device_descriptor() failed: " << libusb_strerror(ret));
  }

  std::cout << "Controller:        " << dev_type.name << std::endl;
  std::cout << "Vendor/Product:    " << std::format("{:04x}:{:04x}",
                                                    uint16_t(desc.idVendor),
                                                    uint16_t(desc.idProduct)) << std::endl;
  std::cout << "USB Path:          " << std::format("{:03d}:{:03d}",
                                                    static_cast<int>(libusb_get_bus_number(dev)),
                                                    static_cast<int>(libusb_get_device_address(dev))) << std::endl;
  if (dev_type.type == GAMEPAD_XBOX360_WIRELESS)
    std::cout << "Wireless Port:     " << opts.wireless_id << std::endl;
  std::cout << "Controller Type:   " << dev_type.type << std::endl;

  //std::cout << "ForceFeedback:     " << ((opts.controller.back().uinput.force_feedback) ? "enabled" : "disabled") << std::endl;
}


void
XboxdrvMain::print_feature_status() const
{
  auto yn = [](bool v) { return v ? "on" : "off"; };
  // Label column width (including trailing spaces after the colon).
  auto line = [](char const* label, std::string const& value) {
    std::cout << "  " << std::left << std::setw(22) << label << value << "\n";
  };

  bool const ff = m_opts.get_controller_slot().get_force_feedback();

  std::cout << "\nFeature status:\n";
  line("uinput:", yn(!m_opts.no_uinput));
  line("force-feedback:",
       std::string(yn(ff)) + (ff ? "" : "  (--force-feedback)"));
  if (m_opts.rumble)
  {
    line("test-rumble:", "on  (--test-rumble)");
  }
  if (m_opts.rumble_l != -1 || m_opts.rumble_r != -1)
  {
    line("startup rumble:",
         std::to_string(m_opts.rumble_l < 0 ? 0 : m_opts.rumble_l) + "," +
         std::to_string(m_opts.rumble_r < 0 ? 0 : m_opts.rumble_r) +
         "  (--rumble L,R)");
  }
  line("chatpad:",
       std::string(yn(m_opts.chatpad)) + (m_opts.chatpad ? "" : "  (--chatpad)"));

  std::string headset;
  if (m_opts.headset_pipewire)
  {
    headset = "pipewire  (--headset-pipewire)";
  }
  else if (m_opts.headset_pulse)
  {
    headset = "pulse/pipe FIFOs  (--headset-pulse)";
  }
  else if (m_opts.headset || !m_opts.headset_pcm.empty() || !m_opts.headset_wav.empty()
           || !m_opts.headset_play_wav.empty() || !m_opts.headset_dump.empty()
           || !m_opts.headset_play.empty())
  {
    headset = "raw/debug  (--headset / --headset-pcm / …)";
  }
  else
  {
    headset = "off  (--headset-pipewire | --headset-pulse | --headset)";
  }
  line("headset:", headset);
  if (m_opts.headset_mic_gain != 1.0f)
  {
    line("headset-mic-gain:", std::to_string(m_opts.headset_mic_gain));
  }
  line("detach-kernel-driver:",
       std::string(yn(m_opts.detach_kernel_driver)) +
       (m_opts.detach_kernel_driver ? "" : "  (--detach-kernel-driver)"));
}

void
XboxdrvMain::print_device_nodes() const
{
  std::cout << "\nController input devices:\n";
  if (m_opts.no_uinput)
  {
    std::cout << "  (none — uinput disabled)\n";
    return;
  }
  if (!m_uinput)
  {
    std::cout << "  (uinput not created)\n";
    return;
  }

  // Brief settle so joydev can create js* nodes under the new input device.
  usleep(50 * 1000);

  auto nodes = m_uinput->collect_device_nodes();
  if (nodes.empty())
  {
    // Old kernels without UI_GET_SYSNAME: last-resort estimate (racy).
    int const js = uinpp::find_jsdev_number();
    int const ev = uinpp::find_evdev_number();
    std::cout << "  /dev/input/js" << js << "  (estimated — could not read sysfs name)\n";
    std::cout << "  /dev/input/event" << ev << "  (estimated — could not read sysfs name)\n";
    return;
  }

  for (std::string const& n : nodes)
  {
    std::cout << "  " << n << "\n";
  }
}

void
XboxdrvMain::shutdown()
{
  log_info("shutdown requested");

  if (!m_controller->is_disconnected())
  {
    m_controller->set_led(0);

    // give the LED message a few msec to reach the controller
    g_usleep(10 * 1000); // FIXME: what is a good time to wait?
  }

  g_main_loop_quit(m_gmain);
}

void
XboxdrvMain::on_sigint(int)
{
  XboxdrvMain::current()->shutdown();
}

} // namespace xboxdrv

/* EOF */
