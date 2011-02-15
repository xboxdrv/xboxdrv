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

#include "xboxdrv_daemon.hpp"

#include <boost/format.hpp>
#include <fstream>
#include <iostream>

#include "uinput_message_processor.hpp"
#include "dummy_message_processor.hpp"
#include "helper.hpp"
#include "raise_exception.hpp"
#include "select.hpp"
#include "uinput.hpp"
#include "usb_helper.hpp"
#include "controller_factory.hpp"
#include "controller_thread.hpp"
#include "controller.hpp"

extern bool global_exit_xboxdrv;

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
    *vendor_id = hexstr2int(vendor_id_str);
  }

  const char* product_id_str = udev_device_get_property_value(device, "ID_MODEL_ID");
  if (!product_id_str)
  {
    return false;
  }
  else
  {
    *product_id = hexstr2int(product_id_str);
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

XboxdrvDaemon::XboxdrvDaemon(const Options& opts) :
  m_opts(opts),
  m_udev(0),
  m_monitor(0),
  m_controller_slots(),
  m_inactive_threads(),
  m_uinput(),
  m_wakeup_pipe()
{
}

XboxdrvDaemon::~XboxdrvDaemon()
{
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    (*i)->disconnect();
  }

  udev_monitor_unref(m_monitor);
  udev_unref(m_udev);
}

void
XboxdrvDaemon::cleanup_threads()
{
  int count = 0;

  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if ((*i)->is_connected() && (*i)->can_disconnect())
    {
      count += 1;
      // FIXME: we kill the thread in try_disconnect() but on_disconnect() needs it
      on_disconnect(*i);
      (*i)->disconnect();
    }
  }

  if (count > 0)
  {
    log_info("cleaned up " << count << " thread(s), free slots: " <<
             get_free_slot_count() << "/" << m_controller_slots.size());
  }
}

void
XboxdrvDaemon::process_match(const Options& opts, struct udev_device* device)
{
  if (g_logger.get_log_level() >= Logger::kDebug)
  {
    print_info(device);
  }

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
          launch_controller_thread(device, dev_type, opts, bus, dev);
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
XboxdrvDaemon::init_uinput(const Options& opts)
{
  // Setup uinput
  if (opts.no_uinput)
  {
    log_info("starting without UInput");

    // just create some empty controller slots
    m_controller_slots.resize(opts.controller_slots.size());
  }
  else
  {
    log_info("starting with UInput");

    m_uinput.reset(new UInput(opts.extra_events));
    m_uinput->set_device_names(opts.uinput_device_names);

    // create controller slots
    int slot_count = 0;

    for(Options::ControllerSlots::const_iterator controller = opts.controller_slots.begin(); 
        controller != opts.controller_slots.end(); ++controller)
    {
      log_info("creating slot: " << slot_count);
      m_controller_slots.push_back(
        ControllerSlotPtr(new ControllerSlot(m_controller_slots.size(),
                                             ControllerSlotConfig::create(*m_uinput, slot_count,
                                                                          opts.extra_devices,
                                                                          controller->second),
                                             controller->second.get_match_rules(),
                                             controller->second.get_led_status())));
      slot_count += 1;
    }

    log_info("created " << m_controller_slots.size() << " controller slots");

    // After all the ControllerConfig registered their events, finish up
    // the device creation
    m_uinput->finish();
  }
}

void
XboxdrvDaemon::init_udev()
{
  assert(!m_udev);

  m_udev = udev_new();
   
  if (!m_udev)
  {
    raise_exception(std::runtime_error, "udev init failure");
  }
}

void
XboxdrvDaemon::init_udev_monitor(const Options& opts)
{
  // Setup udev monitor and enumerate
  m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", "usb_device");
  udev_monitor_enable_receiving(m_monitor);

  // FIXME: won't we get devices twice that have been plugged in at
  // this point? once from the enumeration, once from the monitor
  enumerate_udev_devices(opts);
}

void
XboxdrvDaemon::enumerate_udev_devices(const Options& opts)
{
  // Enumerate over all devices already connected to the computer
  struct udev_enumerate* enumerate = udev_enumerate_new(m_udev);
  assert(enumerate);

  udev_enumerate_add_match_subsystem(enumerate, "usb");
  // not available yet: udev_enumerate_add_match_is_initialized(enumerate);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry* devices;
  struct udev_list_entry* dev_list_entry;
    
  devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry_foreach(dev_list_entry, devices) 
  {
    // name is path, value is NULL
    const char* path = udev_list_entry_get_name(dev_list_entry);

    struct udev_device* device = udev_device_new_from_syspath(m_udev, path);

    // manually filter for devtype, as udev enumerate can't do it by itself
    const char* devtype = udev_device_get_devtype(device);
    if (devtype && strcmp(devtype, "usb_device") == 0)
    {
      process_match(opts, device);
    }
    udev_device_unref(device);
  }
  udev_enumerate_unref(enumerate);
}

void
XboxdrvDaemon::create_pid_file(const Options& opts)
{
  if (!opts.pid_file.empty())
  {
    log_info("writing pid file: " << opts.pid_file);
    std::ofstream out(opts.pid_file.c_str());
    if (!out)
    {
      raise_exception(std::runtime_error, "failed to create pid file: " << opts.pid_file << ": " << strerror(errno));
    }
    else
    {
      out << getpid() << std::endl;
    }
  }
}

void
XboxdrvDaemon::run(const Options& opts)
{
  try 
  {
    create_pid_file(opts);

    init_uinput(opts);
    init_udev();
    init_udev_monitor(opts);

    run_loop(opts);
  }
  catch(const std::exception& err)
  {
    log_error("fatal exception: " << err.what());
  }
}

void
XboxdrvDaemon::check_thread_status()
{
  // check for inactive threads and free the slots
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    // if a slot contains an inactive thread, disconnect it and save
    // the thread for later when it might be active again
    if ((*i)->get_thread() && !(*i)->get_thread()->is_active())
    {
      ControllerThreadPtr thread = disconnect(*i);
      m_inactive_threads.push_back(thread);
    }
  }

  // check for activated threads and connect them to a slot
  for(Threads::iterator i = m_inactive_threads.begin(); i != m_inactive_threads.end(); ++i)
  {
    if (!*i)
    {
      log_error("NULL in m_inactive_threads, shouldn't happen");
    }
    else
    {
      if ((*i)->is_active())
      {
        ControllerSlotPtr slot = find_free_slot(*i);
        if (!slot)
        {
          log_info("couldn't find a free slot for activated controller");
        }
        else
        {
          connect(slot, *i);

          // successfully connected the thread, so set it to NULL and cleanup later
          *i = ControllerThreadPtr();
        }
      }
    }
  }

  // cleanup inactive threads
  m_inactive_threads.erase(std::remove(m_inactive_threads.begin(), m_inactive_threads.end(), ControllerThreadPtr()),
                           m_inactive_threads.end());
}

ControllerSlotPtr
XboxdrvDaemon::find_free_slot(ControllerThreadPtr thread)
{
  const std::vector<ControllerSlotWeakPtr>& slots = thread->get_compatible_slots();
  for(std::vector<ControllerSlotWeakPtr>::const_iterator i = slots.begin(); 
      i != slots.end();
      ++i)
  {
    ControllerSlotPtr slot = i->lock();
    if (!slot->is_connected())
    {
      return slot;
    }
  }
  return ControllerSlotPtr();  
}

void
XboxdrvDaemon::run_loop(const Options& opts)
{
  // 1) when a wakeup event arrives, we check thread status and add
  //    controllers that have become active and remove ones that have
  //    become inactive
  // 2) when new controller arrives via udev, we add it, if no free
  //    slots are found, we ignore it till its plugged out and in
  //    again, this is by design, to avoid situations where replugging
  //    an working controller would result in an previously unused one
  //    grapping its slot, thus we get a more static mapping (however,
  //    still not a perfect one)
  while(!global_exit_xboxdrv)
  {
    log_debug("going to wait in select()");
    Select sel;
    sel.add_fd(udev_monitor_get_fd(m_monitor));
    sel.add_fd(m_wakeup_pipe.get_read_fd());
    sel.wait();
    log_debug("select() is done");

    // cleanup threads that are no longer running
    cleanup_threads();

    if (sel.is_ready(m_wakeup_pipe.get_read_fd()))
    {
      log_debug("got a wakeup call");
      check_thread_status();
    }

    if (sel.is_ready(udev_monitor_get_fd(m_monitor)))
    {
      struct udev_device* device = udev_monitor_receive_device(m_monitor);

      if (!device)
      {
        // seem to be normal, do we get this when the given device is filtered out?
        log_debug("udev device couldn't be read: " << device);
      }
      else
      {
        const char* action = udev_device_get_action(device);

        if (action && strcmp(action, "add") == 0)
        {
          process_match(opts, device);
        }

        udev_device_unref(device);
      }
    }
  }
}

void
XboxdrvDaemon::print_info(struct udev_device* device)
{
  log_debug("/---------------------------------------------");
  log_debug("devpath: " << udev_device_get_devpath(device));
  
  if (udev_device_get_action(device))
    log_debug("action: " << udev_device_get_action(device));
  //log_debug("init: " << udev_device_get_is_initialized(device));

  if (udev_device_get_subsystem(device))
    log_debug("subsystem: " << udev_device_get_subsystem(device));

  if (udev_device_get_devtype(device))
    log_debug("devtype:   " << udev_device_get_devtype(device));

  if (udev_device_get_syspath(device))
    log_debug("syspath:   " << udev_device_get_syspath(device));

  if (udev_device_get_sysname(device))
    log_debug("sysname:   " << udev_device_get_sysname(device));

  if (udev_device_get_sysnum(device))
    log_debug("sysnum:    " << udev_device_get_sysnum(device));

  if (udev_device_get_devnode(device))
    log_debug("devnode:   " << udev_device_get_devnode(device));

  if (udev_device_get_driver(device))
    log_debug("driver:    " << udev_device_get_driver(device));

  if (udev_device_get_action(device))
    log_debug("action:    " << udev_device_get_action(device));
          
  //udev_device_get_sysattr_value(device, "busnum");
  //udev_device_get_sysattr_value(device, "devnum");

#if 0
  // FIXME: only works with newer versions of libudev
  {
    log_debug("list: ");
    struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {         
      log_debug("  " 
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }
          
  {
    log_debug("properties: ");
    struct udev_list_entry* it = udev_device_get_properties_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {         
      log_debug("  " 
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }
          
  {
    log_debug("devlist: ");
    struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
    while((it = udev_list_entry_get_next(it)) != 0)
    {         
      log_debug("  " 
                << udev_list_entry_get_name(it) << " = "
                << udev_list_entry_get_value(it)
        );
    }
  }
#endif

  log_debug("\\----------------------------------------------");
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

std::vector<ControllerSlotPtr>
XboxdrvDaemon::find_compatible_slots(udev_device* dev)
{
  std::vector<ControllerSlotPtr> lst;
  
  // add all slots with matching rules
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    for(std::vector<ControllerMatchRulePtr>::const_iterator rule = (*i)->get_rules().begin(); 
        rule != (*i)->get_rules().end(); ++rule)
    {
      if ((*rule)->match(dev))
      {
        lst.push_back(*i);
      }
    }
  }

  // add all slots without any rules at all (i.e. match everything)
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if ((*i)->get_rules().empty())
    {
      lst.push_back(*i);
    }
  }
    
  return lst;
}

void
XboxdrvDaemon::launch_controller_thread(udev_device* udev_dev,
                                        const XPadDevice& dev_type, const Options& opts, 
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
    std::vector<ControllerPtr> controllers = ControllerFactory::create_multiple(dev_type, dev, opts);
    for(std::vector<ControllerPtr>::iterator i = controllers.begin();
        i != controllers.end(); 
        ++i)
    {
      ControllerPtr& controller = *i;

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
          std::auto_ptr<MessageProcessor> message_proc;
          if (m_uinput.get())
          {
            message_proc.reset(new UInputMessageProcessor(*m_uinput, slot->get_config(), opts));
          }
          else
          {
            message_proc.reset(new DummyMessageProcessor());
          }

          ControllerThreadPtr thread(new ControllerThread(controller, opts));
          thread->set_message_proc(message_proc); 
          connect(slot, thread);
          thread->start_thread(opts);
        }
      }
      else // if (!controller->is_active())
      {
        controller->set_activation_cb(boost::bind(&XboxdrvDaemon::wakeup, this));

        // controller is inactive, so put it on the back log till it gets active
        ControllerThreadPtr thread(new ControllerThread(controller, opts));
        thread->set_compatible_slots(find_compatible_slots(udev_dev));
        thread->start_thread(opts);
        m_inactive_threads.push_back(thread);
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
XboxdrvDaemon::connect(ControllerSlotPtr slot, ControllerThreadPtr thread)
{
  log_info("connecting slot to thread");

  {
    // set the LED status
    if (slot->get_led_status() == -1)
    {
      thread->get_controller()->set_led(2 + (slot->get_id() % 4));
    }
    else
    {
      thread->get_controller()->set_led(slot->get_led_status());
    }
  }
  
  {
    // connect thread with the current slots message proc
    // FIXME: Could get rid of MessageProcessor and just use
    // ControllerSlot?! Or a callback to ControllerSlot.
    std::auto_ptr<MessageProcessor> message_proc;
    if (m_uinput.get())
    {
      message_proc.reset(new UInputMessageProcessor(*m_uinput, slot->get_config(), m_opts));
    }
    else
    {
      message_proc.reset(new DummyMessageProcessor());
    }
    thread->set_message_proc(message_proc);
  }
  
  slot->connect(thread);
  on_connect(slot);

  log_info("controller connected: " 
           << thread->get_usbpath() << " "
           << thread->get_usbid() << " "
           << "'" << thread->get_name() << "'");

  log_info("launched ControllerThread for " << thread->get_usbpath()
           << " in slot " << slot->get_id() << ", free slots: " 
           << get_free_slot_count() << "/" << m_controller_slots.size());
}

ControllerThreadPtr
XboxdrvDaemon::disconnect(ControllerSlotPtr slot)
{
  ControllerThreadPtr thread = slot->disconnect();
  on_disconnect(slot);
  return thread;
}

void
XboxdrvDaemon::on_connect(ControllerSlotPtr slot)
{
  ControllerThreadPtr thread = slot->get_thread();
  assert(thread);

  if (!m_opts.on_connect.empty())
  {
    log_info("launching connect script: " << m_opts.on_connect);

    std::vector<std::string> args;
    args.push_back(m_opts.on_connect);
    args.push_back(thread->get_usbpath());
    args.push_back(thread->get_usbid());
    args.push_back(thread->get_name());
    spawn_exe(args);
  }
}

void
XboxdrvDaemon::on_disconnect(ControllerSlotPtr slot)
{
  ControllerThreadPtr thread = slot->get_thread();
  assert(thread);

  log_info("controller disconnected: " 
           << thread->get_usbpath() << " "
           << thread->get_usbid() << " "
           << "'" << thread->get_name() << "'");

  if (!m_opts.on_disconnect.empty())
  {
    log_info("launching disconnect script: " << m_opts.on_disconnect);

    std::vector<std::string> args;
    args.push_back(m_opts.on_disconnect);
    args.push_back(thread->get_usbpath());
    args.push_back(thread->get_usbid());
    args.push_back(thread->get_name());
    spawn_exe(args);
  }
}

void
XboxdrvDaemon::wakeup()
{
  log_info("received wakeup call");
  m_wakeup_pipe.send_wakeup();
}

/* EOF */
