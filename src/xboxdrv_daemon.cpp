/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include "xboxdrv_daemon.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <fstream>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>
#include <errno.h>

#include "helper.hpp"
#include "raise_exception.hpp"
#include "select.hpp"
#include "uinput.hpp"
#include "usb_helper.hpp"
#include "usb_gsource.hpp"
#include "controller_factory.hpp"
#include "controller_slot.hpp"
#include "controller.hpp"
#include "udev_subsystem.hpp"
#include "dbus_subsystem.hpp"
#include "usb_subsystem.hpp"

XboxdrvDaemon* XboxdrvDaemon::s_current = 0;

namespace {

bool get_usb_id(udev_device* device, uint16_t* vendor_id, uint16_t* product_id)
{
  const char* vendor_id_str  = udev_device_get_property_value(device, "ID_VENDOR_ID");
  if (!vendor_id_str)
  {
    return false;
  }
  else
  {
    *vendor_id = hexstr2uint16(vendor_id_str);
  }

  const char* product_id_str = udev_device_get_property_value(device, "ID_MODEL_ID");
  if (!product_id_str)
  {
    return false;
  }
  else
  {
    *product_id = hexstr2uint16(product_id_str);
  }

  return true;
}

bool get_usb_path(udev_device* device, int* bus, int* dev)
{
  // busnum:devnum are decimal, not hex
  const char* busnum_str = udev_device_get_property_value(device, "BUSNUM");
  if (!busnum_str)
  {
    return false;
  }
  else
  {
    *bus = boost::lexical_cast<int>(busnum_str);
  }

  const char* devnum_str = udev_device_get_property_value(device, "DEVNUM");
  if (!devnum_str)
  {
    return false;
  }
  else
  {
    *dev = boost::lexical_cast<int>(devnum_str);
  }

  return true;
}

} // namespace

XboxdrvDaemon::XboxdrvDaemon(USBSubsystem& usb_subsystem, const Options& opts) :
  m_usb_subsystem(usb_subsystem),
  m_opts(opts),
  m_gmain(),
  m_controller_slots(),
  m_inactive_controllers(),
  m_uinput()
{
  assert(!s_current);
  s_current = this;

#if !GLIB_CHECK_VERSION(2,35,0)
  g_type_init();
#endif

  m_gmain = g_main_loop_new(NULL, false);

  signal(SIGINT,  &XboxdrvDaemon::on_sigint);
  signal(SIGTERM, &XboxdrvDaemon::on_sigint);
}

XboxdrvDaemon::~XboxdrvDaemon()
{
  signal(SIGINT,  NULL);
  signal(SIGTERM, NULL);

  assert(s_current);
  s_current = 0;

  g_main_loop_unref(m_gmain);
}

void
XboxdrvDaemon::run()
{
  try
  {
    create_pid_file();

    init_uinput();

    UdevSubsystem udev_subsystem;
    udev_subsystem.set_device_callback(boost::bind(&XboxdrvDaemon::process_match, this, _1));

    boost::scoped_ptr<DBusSubsystem> dbus_subsystem;
    if (m_opts.dbus != Options::kDBusDisabled)
    {
      DBusBusType dbus_bus_type;

      switch(m_opts.dbus)
      {
        case Options::kDBusAuto:
          if (getuid() == 0)
          {
            dbus_bus_type = DBUS_BUS_SYSTEM;
          }
          else
          {
            dbus_bus_type = DBUS_BUS_SESSION;
          }
          break;

        case Options::kDBusSession:
          dbus_bus_type = DBUS_BUS_SESSION;
          break;

        case Options::kDBusSystem:
          dbus_bus_type = DBUS_BUS_SYSTEM;
          break;

        case Options::kDBusDisabled:
        default:
          assert(!"should never happen");
          break;
      }

      dbus_subsystem.reset(new DBusSubsystem("org.seul.Xboxdrv", dbus_bus_type));
      dbus_subsystem->register_xboxdrv_daemon(this);
      dbus_subsystem->register_controller_slots(m_controller_slots);
    }

    log_debug("launching into main loop");
    g_main_loop_run(m_gmain);
    log_debug("main loop exited");

    // get rid of active ControllerThreads before the subsystems shutdown
    m_inactive_controllers.clear();
    m_controller_slots.clear();
  }
  catch(const std::exception& err)
  {
    log_error("fatal exception: " << err.what());
  }
}

void
XboxdrvDaemon::process_match(struct udev_device* device)
{
  // FIXME: bad place?!
  // FIXME: cleanup_threads();

  uint16_t vendor;
  uint16_t product;

  if (!get_usb_id(device, &vendor, &product))
  {
    log_warn("couldn't get vendor:product, ignoring device");
  }
  else
  {
    XPadDevice dev_type;
    if (!find_xpad_device(vendor, product, &dev_type))
    {
      log_debug("ignoring " << boost::format("%04x:%04x") % vendor % product <<
                " not a valid Xboxdrv device");
    }
    else
    {
      int bus;
      int dev;
      if (!get_usb_path(device, &bus, &dev))
      {
        log_warn("couldn't get bus:dev");
      }
      else
      {
        try
        {
          launch_controller_thread(device, dev_type, static_cast<uint8_t>(bus), static_cast<uint8_t>(dev));
        }
        catch(const std::exception& err)
        {
          log_error("failed to launch ControllerThread: " << err.what());
        }
      }
    }
  }
}

void
XboxdrvDaemon::init_uinput()
{
  // Setup uinput
  if (m_opts.no_uinput)
  {
    log_info("starting without UInput");

    // just create some empty controller slots
    m_controller_slots.resize(m_opts.controller_slots.size());
  }
  else
  {
    log_info("starting with UInput");

    m_uinput.reset(new UInput(m_opts.extra_events));
    m_uinput->set_device_names(m_opts.uinput_device_names);

    // create controller slots
    int slot_count = 0;

    for(Options::ControllerSlots::const_iterator controller = m_opts.controller_slots.begin();
        controller != m_opts.controller_slots.end(); ++controller)
    {
      log_info("creating slot: " << slot_count);
      m_controller_slots.push_back(
        ControllerSlotPtr(new ControllerSlot(m_controller_slots.size(),
                                             ControllerSlotConfig::create(*m_uinput, slot_count,
                                                                          m_opts.extra_devices,
                                                                          controller->second),
                                             controller->second.get_match_rules(),
                                             controller->second.get_led_status(),
                                             m_opts)));
      slot_count += 1;
    }

    log_info("created " << m_controller_slots.size() << " controller slots");

    // After all the ControllerConfig registered their events, finish up
    // the device creation
    m_uinput->finish();
  }
}

void
XboxdrvDaemon::create_pid_file()
{
  if (!m_opts.pid_file.empty())
  {
    log_info("writing pid file: " << m_opts.pid_file);
    std::ofstream out(m_opts.pid_file.c_str());
    if (!out)
    {
      raise_exception(std::runtime_error, "failed to create pid file: " << m_opts.pid_file << ": " << strerror(errno));
    }
    else
    {
      out << getpid() << std::endl;
    }
  }
}

ControllerSlotPtr
XboxdrvDaemon::find_free_slot(udev_device* dev)
{
  // first pass, look for slots where the rules match the given vendor:product, bus:dev
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if (!(*i)->is_connected())
    {
      // found a free slot, check if the rules match
      for(std::vector<ControllerMatchRulePtr>::const_iterator rule = (*i)->get_rules().begin();
          rule != (*i)->get_rules().end(); ++rule)
      {
        if ((*rule)->match(dev))
        {
          return *i;
        }
      }
    }
  }

  // second pass, look for slots that don't have any rules and thus match everything
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if (!(*i)->is_connected() && (*i)->get_rules().empty())
    {
      return *i;
    }
  }

  // no free slot found
  return ControllerSlotPtr();
}

void
XboxdrvDaemon::launch_controller_thread(udev_device* udev_dev,
                                        const XPadDevice& dev_type,
                                        uint8_t busnum, uint8_t devnum)
{
  // FIXME: results must be libusb_unref_device()'ed
  libusb_device* dev = usb_find_device_by_path(busnum, devnum);

  if (!dev)
  {
    log_error("USB device disappeared before it could be opened");
  }
  else
  {
    std::vector<ControllerPtr> controllers = ControllerFactory::create_multiple(dev_type, dev, m_opts);
    for(std::vector<ControllerPtr>::iterator i = controllers.begin();
        i != controllers.end();
        ++i)
    {
      ControllerPtr& controller = *i;

      controller->set_disconnect_cb(boost::bind(&g_idle_add, &XboxdrvDaemon::on_controller_disconnect_wrap, this));
      controller->set_activation_cb(boost::bind(&g_idle_add, &XboxdrvDaemon::on_controller_activate_wrap, this));

      // FIXME: Little dirty hack
      controller->set_udev_device(udev_dev);

      if (controller->is_active())
      {
        // controller is active, so launch a thread if we have a free slot
        ControllerSlotPtr slot = find_free_slot(udev_dev);
        if (!slot)
        {
          log_error("no free controller slot found, controller will be ignored: "
                    << boost::format("%03d:%03d %04x:%04x '%s'")
                    % static_cast<int>(busnum)
                    % static_cast<int>(devnum)
                    % dev_type.idVendor
                    % dev_type.idProduct
                    % dev_type.name);
        }
        else
        {
          connect(slot, controller);
        }
      }
      else // if (!controller->is_active())
      {
        m_inactive_controllers.push_back(controller);
      }
    }
  }
}

int
XboxdrvDaemon::get_free_slot_count() const
{
  int slot_count = 0;

  for(ControllerSlots::const_iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if (!(*i)->is_connected())
    {
      slot_count += 1;
    }
  }

  return slot_count;
}

void
XboxdrvDaemon::connect(ControllerSlotPtr slot, ControllerPtr controller)
{
  log_info("connecting slot to thread");

  try
  {
    // set the LED status
    if (slot->get_led_status() == -1)
    {
      controller->set_led(static_cast<uint8_t>(2 + (slot->get_id() % 4)));
    }
    else
    {
      controller->set_led(static_cast<uint8_t>(slot->get_led_status()));
    }
  }
  catch(const std::exception& err)
  {
    log_error("failed to set led: " << err.what());
  }

  slot->connect(controller);
  on_connect(slot);

  log_info("controller connected: "
           << controller->get_usbpath() << " "
           << controller->get_usbid() << " "
           << "'" << controller->get_name() << "'");

  log_info("launched Controller for " << controller->get_usbpath()
           << " in slot " << slot->get_id() << ", free slots: "
           << get_free_slot_count() << "/" << m_controller_slots.size());
}

ControllerPtr
XboxdrvDaemon::disconnect(ControllerSlotPtr slot)
{
  on_disconnect(slot);
  return slot->disconnect();
}

void
XboxdrvDaemon::on_connect(ControllerSlotPtr slot)
{
  ControllerPtr controller = slot->get_controller();
  assert(controller);

  if (!m_opts.on_connect.empty())
  {
    log_info("launching connect script: " << m_opts.on_connect);

    std::vector<std::string> args;
    args.push_back(m_opts.on_connect);
    args.push_back(controller->get_usbpath());
    args.push_back(controller->get_usbid());
    args.push_back(controller->get_name());
    spawn_exe(args);
  }
}

void
XboxdrvDaemon::on_disconnect(ControllerSlotPtr slot)
{
  ControllerPtr controller = slot->get_controller();
  assert(controller);

  log_info("controller disconnected: "
           << controller->get_usbpath() << " "
           << controller->get_usbid() << " "
           << "'" << controller->get_name() << "'");

  if (!m_opts.on_disconnect.empty())
  {
    log_info("launching disconnect script: " << m_opts.on_disconnect);

    std::vector<std::string> args;
    args.push_back(m_opts.on_disconnect);
    args.push_back(controller->get_usbpath());
    args.push_back(controller->get_usbid());
    args.push_back(controller->get_name());
    spawn_exe(args);
  }
}

void
XboxdrvDaemon::on_controller_disconnect()
{
  //log_tmp("on_controller_disconnect");

  // cleanup active controllers in slots
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if ((*i)->get_controller() && (*i)->get_controller()->is_disconnected())
    {
      disconnect(*i); // discard the ControllerPtr
    }
  }

  // cleanup inactive controllers
  m_inactive_controllers.erase(std::remove_if(m_inactive_controllers.begin(), m_inactive_controllers.end(),
                                              boost::bind(&Controller::is_disconnected, _1)),
                               m_inactive_controllers.end());
}

void
XboxdrvDaemon::on_controller_activate()
{
  //log_tmp("on_controller_activate");

  // check for inactive controller and free the slots
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    // if a slot contains an inactive controller, disconnect it and save
    // the controller for later when it might be active again
    if ((*i)->get_controller() && !(*i)->get_controller()->is_active())
    {
      ControllerPtr controller = disconnect(*i);
      m_inactive_controllers.push_back(controller);
    }
  }

  // check for activated controller and connect them to a slot
  for(Controllers::iterator i = m_inactive_controllers.begin(); i != m_inactive_controllers.end(); ++i)
  {
    if (!*i)
    {
      log_error("NULL in m_inactive_controllers, shouldn't happen");
    }
    else
    {
      if ((*i)->is_active())
      {
        ControllerSlotPtr slot = find_free_slot((*i)->get_udev_device());
        if (!slot)
        {
          log_info("couldn't find a free slot for activated controller");
        }
        else
        {
          connect(slot, *i);

          // successfully connected the controller, so set it to NULL and cleanup later
          *i = ControllerPtr();
        }
      }
    }
  }

  // cleanup inactive controller
  m_inactive_controllers.erase(std::remove(m_inactive_controllers.begin(), m_inactive_controllers.end(),
                                           ControllerPtr()),
                               m_inactive_controllers.end());
}

std::string
XboxdrvDaemon::status()
{
  std::ostringstream out;

  out << boost::format("SLOT  CFG  NCFG    USBID    USBPATH  NAME\n");
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if ((*i)->get_controller())
    {
      out << boost::format("%4d  %3d  %4d  %5s  %7s  %s\n")
        % (i - m_controller_slots.begin())
        % (*i)->get_config()->get_current_config()
        % (*i)->get_config()->config_count()
        % (*i)->get_controller()->get_usbid()
        % (*i)->get_controller()->get_usbpath()
        % (*i)->get_controller()->get_name();
    }
    else
    {
      out << boost::format("%4d  %3d  %4d      -         -\n")
        % (i - m_controller_slots.begin())
        % (*i)->get_config()->get_current_config()
        % (*i)->get_config()->config_count();
    }
  }

  for(Controllers::iterator i = m_inactive_controllers.begin(); i != m_inactive_controllers.end(); ++i)
  {
    out << boost::format("   -             %5s  %7s  %s\n")
      % (*i)->get_usbid()
      % (*i)->get_usbpath()
      % (*i)->get_name();
  }

  return out.str();
}

void
XboxdrvDaemon::shutdown()
{
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if ((*i)->get_controller() &&
        !(*i)->get_controller()->is_disconnected())
    {
      (*i)->get_controller()->set_led(0);
    }
  }

  // give the LED message a few msec to reach the controller
  g_usleep(10 * 1000); // FIXME: what is a good time to wait?

  assert(m_gmain);
  g_main_loop_quit(m_gmain);
}

void
XboxdrvDaemon::on_sigint(int)
{
  XboxdrvDaemon::current()->shutdown();
}

/* EOF */
