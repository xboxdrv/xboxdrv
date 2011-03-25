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

#include "controller_factory.hpp"
#include "evdev_controller.hpp"
#include "message_processor.hpp"
#include "uinput_message_processor.hpp"
#include "dummy_message_processor.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"
#include "usb_helper.hpp"
#include "usb_gsource.hpp"
#include "usb_subsystem.hpp"
#include "controller_thread.hpp"
#include "helper.hpp"

XboxdrvMain::XboxdrvMain(const Options& opts) :
  m_opts(opts),
  m_gmain(),
  m_usb_gsource(),
  m_uinput(),
  m_jsdev_number(),
  m_evdev_number(),
  m_use_libusb(false),
  m_dev_type()
{
  m_gmain = g_main_loop_new(NULL, false);
}

XboxdrvMain::~XboxdrvMain()
{
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
                                             m_opts.evdev_grab,
                                             m_opts.evdev_debug));

    // FIXME: ugly, should be part of Controller
    m_dev_type.type = GAMEPAD_XBOX360;
    m_dev_type.idVendor  = 0;
    m_dev_type.idProduct = 0;
    m_dev_type.name = "Evdev device";
  }
  else
  { // regular USB Xbox360 controller    

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
    controller->set_led(2 + m_jsdev_number % 4);
  }
  else
  {
    controller->set_led(m_opts.get_controller_slot().get_led_status());
  }

  if (m_opts.rumble_l != -1 && m_opts.rumble_r != -1)
  { // Only set rumble when explicitly requested
    controller->set_rumble(m_opts.rumble_l, m_opts.rumble_r);
  }
}

void
XboxdrvMain::run()
{
  USBSubsystem usb_subsystem;

  ControllerPtr controller = create_controller();
  std::auto_ptr<MessageProcessor> message_proc;
  init_controller(controller);

  if (!m_opts.quiet)
    std::cout << std::endl;
      
  if (m_opts.instant_exit)
  {
    usleep(1000);
  }
  else
  {          
    if (m_opts.no_uinput)
    {
      if (!m_opts.quiet)
      {
        std::cout << "Starting without uinput" << std::endl;
      }

      message_proc.reset(new DummyMessageProcessor);
    }
    else
    {
      log_debug("creating UInput");
      m_uinput.reset(new UInput(m_opts.extra_events));
      m_uinput->set_device_names(m_opts.uinput_device_names);
      m_uinput->set_device_usbids(m_opts.uinput_device_usbids);

      log_debug("creating ControllerSlotConfig");
      ControllerSlotConfigPtr config_set = ControllerSlotConfig::create(*m_uinput, 
                                                                        0, m_opts.extra_devices,
                                                                        m_opts.get_controller_slot());
      
      // After all the ControllerConfig registered their events, finish up
      // the device creation
      log_debug("finish UInput creation");
      m_uinput->finish();
      
      message_proc.reset(new UInputMessageProcessor(*m_uinput, config_set, m_opts));
    }

    if (!m_opts.quiet)
    {
      std::cout << "\nYour Xbox/Xbox360 controller should now be available as:" << std::endl
                << "  /dev/input/js" << m_jsdev_number << std::endl
                << "  /dev/input/event" << m_evdev_number << std::endl;

      if (m_opts.silent)
      {
        std::cout << "\nPress Ctrl-c to quit\n" << std::endl;
      }
      else
      {
        std::cout << "\nPress Ctrl-c to quit, use '--silent' to suppress the event output\n" << std::endl;
      }
    }

    {
      ControllerThread thread(controller, message_proc, m_opts);
      log_debug("launching thread");
      
      pid_t pid = 0;
      if (!m_opts.exec.empty())
      {
        pid = spawn_exe(m_opts.exec);
      }

      log_debug("launching main loop");
      g_main_loop_run(m_gmain);
    }

    if (!m_opts.quiet)
    {
      std::cout << "Shutdown complete" << std::endl;
    }
  }
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

/* EOF */
