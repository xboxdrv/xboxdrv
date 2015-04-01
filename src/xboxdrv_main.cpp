/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include <glib.h>
#include <stdio.h>
#include <libusb.h>
#include <stdexcept>
#include <iostream>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include "controller_factory.hpp"
#include "controller_slot_config.hpp"
#include "controller/evdev_controller.hpp"
#include "controller/wiimote_controller.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"
#include "usb_helper.hpp"
#include "usb_gsource.hpp"
#include "usb_subsystem.hpp"
#include "controller_thread.hpp"
#include "helper.hpp"

XboxdrvMain* XboxdrvMain::s_current = 0;

XboxdrvMain::XboxdrvMain(USBSubsystem& usb_subsystem, const Options& opts) :
  m_usb_subsystem(usb_subsystem),
  m_opts(opts),
  m_gmain(),
  m_usb_gsource(),
  m_uinput(),
  m_jsdev_number(),
  m_evdev_number(),
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

  s_current = 0;

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
    log_tmp("Creating Wiimote controller");
    return ControllerPtr(new WiimoteController);
#else
    throw std::runtime_error("libcwiid not found at compile time, Wiimote support is not available");
#endif
  }
  else
  { // regular USB Xbox360-like controller

    // FIXME: this must be libusb_unref_device()'ed, child code must not keep a copy around
    libusb_device* dev = 0;

    USBSubsystem::find_controller(&dev, m_dev_type, m_opts);

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
XboxdrvMain::init_controller(const ControllerPtr& controller)
{
  m_jsdev_number = UInput::find_jsdev_number();
  m_evdev_number = UInput::find_evdev_number();

  if (m_opts.get_controller_slot().get_led_status() == -1)
  {
    controller->set_led(static_cast<uint8_t>(2 + m_jsdev_number % 4));
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
  m_controller->set_disconnect_cb(boost::bind(&XboxdrvMain::on_controller_disconnect, this));
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
      m_uinput.reset(new UInput(m_opts.extra_events));
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
      std::cout << "\nYour Xbox/Xbox360 controller should now be available as:" << std::endl
                << "  /dev/input/js" << m_jsdev_number << std::endl
                << "  /dev/input/event" << m_evdev_number << std::endl;

      if (m_opts.silent)
      {
        std::cout << "\nPress Ctrl-c to quit" << std::endl;
      }
      else
      {
        std::cout << "\nPress Ctrl-c to quit, use '--silent' to suppress the event output" << std::endl;
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
  log_info("child processes exited with status: " << status);
  shutdown();
}

void
XboxdrvMain::print_info(libusb_device* dev, const XPadDevice& dev_type, const Options& opts) const
{
  libusb_device_descriptor desc;
  int ret = libusb_get_device_descriptor(dev, &desc);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_get_device_descriptor() failed: " << usb_strerror(ret));
  }

  std::cout << "Controller:        " << dev_type.name << std::endl;
  std::cout << "Vendor/Product:    " << boost::format("%04x:%04x")
    % uint16_t(desc.idVendor) % uint16_t(desc.idProduct) << std::endl;
  std::cout << "USB Path:          " << boost::format("%03d:%03d")
    % static_cast<int>(libusb_get_bus_number(dev))
    % static_cast<int>(libusb_get_device_address(dev)) << std::endl;
  if (dev_type.type == GAMEPAD_XBOX360_WIRELESS)
    std::cout << "Wireless Port:     " << opts.wireless_id << std::endl;
  std::cout << "Controller Type:   " << dev_type.type << std::endl;

  //std::cout << "ForceFeedback:     " << ((opts.controller.back().uinput.force_feedback) ? "enabled" : "disabled") << std::endl;
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

/* EOF */
