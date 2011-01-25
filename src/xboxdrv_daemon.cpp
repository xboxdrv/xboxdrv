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

#include "default_message_processor.hpp"
#include "dummy_message_processor.hpp"
#include "log.hpp"
#include "uinput.hpp"
#include "usb_helper.hpp"
#include "xbox_controller_factory.hpp"
#include "xboxdrv_thread.hpp"

extern bool global_exit_xboxdrv;

XboxdrvDaemon::XboxdrvDaemon() :
  m_udev(0),
  m_monitor(0),
  m_controller_slots()
{
  m_udev = udev_new();
    
  if (!m_udev)
  {
    throw std::runtime_error("udev init failure");
  }
  else
  {
    // do nothing, stuff is done in run()
  }
}

XboxdrvDaemon::~XboxdrvDaemon()
{
  for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    delete i->thread;
    i->thread = 0;
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
    if (i->thread)
    {
      if (i->thread->try_join_thread())
      {
        delete i->thread;
        i->thread = 0;
        count += 1;
      }
    }
  }

  if (count > 0)
  {
    log_info << "cleaned up " << count << " thread(s), free slots: " 
             << get_free_slot_count() << "/" << m_controller_slots.size()
             << std::endl;
  }
}

void
XboxdrvDaemon::process_match(const Options& opts, UInput* uinput, struct udev_device* device)
{
  // 1) Match vendor/product against the xpad list
  // value = udev_device_get_property_value(device, "ID_VENDOR_ID"); // 045e
  // value = udev_device_get_property_value(device, "ID_MODEL_ID");  // 028e
  // value = udev_device_get_property_value(device, "ID_REVISION");  // 0110 aka bcd
  // PRODUCT = "45e/28e/110"

  const char* product_str = udev_device_get_property_value(device, "PRODUCT");
  if (product_str)
  {
    unsigned int vendor = 0;
    unsigned int product = 0; 
    unsigned int bcd = 0;
    if (sscanf(product_str, "%x/%x/%x", &vendor, &product, &bcd) != 3)
    {
      std::cout << "[XboxdrvDaemon] couldn't parse PRODUCT = " << product_str << std::endl;
    }
    else
    {
      if (false)
        std::cout << "Product parse: " 
                  << boost::format("%03x/%03x/%03x == %s") % vendor % product % bcd % product_str
                  << std::endl;

      // FIXME: could do this after we know that vendor/product are good
      // 2) Get busnum and devnum        
      // busnum:devnum are decimal, not hex
      const char* busnum_str = udev_device_get_property_value(device, "BUSNUM");
      const char* devnum_str = udev_device_get_property_value(device, "DEVNUM");

      if (busnum_str && devnum_str)
      {
        try 
        {
          XPadDevice dev_type;
          if (find_xpad_device(vendor, product, &dev_type))
          {
            // 3) Launch thread to handle the device
            log_info << "controller detected at " << busnum_str << ":" << devnum_str << std::endl;
           
            try 
            {
              launch_xboxdrv(uinput,
                             dev_type, opts,
                             boost::lexical_cast<int>(busnum_str),
                             boost::lexical_cast<int>(devnum_str));
            }
            catch(const std::exception& err)
            {
              log_error << "failed to launch XboxdrvThread: " << err.what() << std::endl;
            }
          }
        }
        catch(const std::exception& err)
        {
          log_error << "child thread lauch failure: " << err.what() << std::endl;
        }
      }
    }
  }  
}

void
XboxdrvDaemon::run(const Options& opts)
{
  try 
  {
    run_real(opts);
  }
  catch(const std::exception& err)
  {
    log_error << "fatal exception: " << err.what() << std::endl;
  }
}

void
XboxdrvDaemon::run_real(const Options& opts)
{
  if (!opts.pid_file.empty())
  {
    log_info << "writing pid file: " << opts.pid_file << std::endl;
    std::ofstream out(opts.pid_file.c_str());
    if (!out)
    {
      std::ostringstream str;
      str << opts.pid_file << ": " << strerror(errno);
      throw std::runtime_error(str.str());
    }
    else
    {
      out << getpid() << std::endl;
    }
  }

  // Setup uinput
  std::auto_ptr<UInput> uinput;
  if (!opts.no_uinput)
  {
    log_info << "starting with UInput" << std::endl;

    uinput.reset(new UInput());

    // FIXME:
    /* must setup this callback later when we have a controller
       if (opts.uinput_config.force_feedback)
       {
       uinput->set_ff_callback(boost::bind(&set_rumble,  controller.get(), opts.rumble_gain, _1, _2));
       }
    */
  }
  else
  {
    log_info << "starting without UInput" << std::endl;
  }

  if (uinput.get())
  {
    // create controller slots
    int slot_count = 0;

    for(Options::ControllerSlots::const_iterator controller = opts.controller_slots.begin(); 
        controller != opts.controller_slots.end(); ++controller)
    {
      log_info << "creating slot: " << slot_count << std::endl;
      m_controller_slots.push_back(ControllerSlot(ControllerConfigSet::create(*uinput, slot_count,
                                                                              opts.extra_devices,
                                                                              controller->second),
                                                  controller->second.get_match_rules()));
      slot_count += 1;
    }

    log_info << "created " << m_controller_slots.size() << " controller slots" << std::endl;

    // After all the ControllerConfig registered their events, finish up
    // the device creation
    uinput->finish();
  }
  else
  {
    // just create some empty controller slots
    m_controller_slots.resize(opts.controller_slots.size());
  }

  // Setup udev monitor and enumerate
  m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
  udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", "usb_device");
  udev_monitor_enable_receiving(m_monitor);

  // FIXME: won't we get devices twice that have been plugged in at
  // this point? once from the enumeration, once from the monitor

  // Enumerate over all devices already connected to the computer
  {
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
      const char* path  = udev_list_entry_get_name(dev_list_entry) ;
      //const char* value = udev_list_entry_get_value(dev_list_entry);
      
      //std::cout << "Enum: " << path << std::endl;

      struct udev_device* device = udev_device_new_from_syspath(m_udev, path);
      process_match(opts, uinput.get(), device);
      udev_device_unref(device);
    }
    udev_enumerate_unref(enumerate);
  }

  while(!global_exit_xboxdrv)
  {
    // FIXME: udev_monitor_receive_device() will block, must break out of it somehow
    // FIXME: we bust udev_unref_monitor() this
    struct udev_device* device = udev_monitor_receive_device(m_monitor);

    cleanup_threads();

    if (!device)
    {
      // seem to be normal, do we get this when the given device is filtered out?
      std::cout << "udev device couldn't be read: " << device << std::endl;
    }
    else
    {
      const char* action = udev_device_get_action(device);

      if (action && strcmp(action, "add") == 0)
      {
        process_match(opts, uinput.get(), device);
      }
    }
    udev_device_unref(device);
  }
}

void
XboxdrvDaemon::print_info(struct udev_device* device)
{
  std::cout << "/---------------------------------------------" << std::endl;
  std::cout << "devpath: " << udev_device_get_devpath(device) << std::endl;
  std::cout << "action: " << udev_device_get_action(device) << std::endl;
  //std::cout << "init: " << udev_device_get_is_initialized(device) << std::endl;

  if (strcmp(udev_device_get_action(device), "add") == 0)
  {
    if (udev_device_get_subsystem(device))
      std::cout << "subsystem: " << udev_device_get_subsystem(device) << std::endl;

    if (udev_device_get_devtype(device))
      std::cout << "devtype:   " << udev_device_get_devtype(device) << std::endl;

    if (udev_device_get_syspath(device))
      std::cout << "syspath:   " << udev_device_get_syspath(device) << std::endl;

    if (udev_device_get_sysname(device))
      std::cout << "sysname:   " << udev_device_get_sysname(device) << std::endl;

    if (udev_device_get_sysnum(device))
      std::cout << "sysnum:    " << udev_device_get_sysnum(device) << std::endl;

    if (udev_device_get_devnode(device))
      std::cout << "devnode:   " << udev_device_get_devnode(device) << std::endl;

    if (udev_device_get_driver(device))
      std::cout << "driver:    " << udev_device_get_driver(device) << std::endl;

    if (udev_device_get_action(device))
      std::cout << "action:    " << udev_device_get_action(device) << std::endl;
          
    udev_device_get_sysattr_value(device, "busnum");
    udev_device_get_sysattr_value(device, "devnum");

    {
      std::cout << "list: " << std::endl;
      struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
          
    {
      std::cout << "properties: " << std::endl;
      struct udev_list_entry* it = udev_device_get_properties_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
          
    {
      std::cout << "devlist: " << std::endl;
      struct udev_list_entry* it = udev_device_get_tags_list_entry(device);
      while((it = udev_list_entry_get_next(it)) != 0)
      {         
        std::cout << "  " 
                  << udev_list_entry_get_name(it) << " = "
                  << udev_list_entry_get_value(it)
                  << std::endl;
      }
    }
  }
  std::cout << "\\----------------------------------------------" << std::endl;
}

void
XboxdrvDaemon::launch_xboxdrv(UInput* uinput, const XPadDevice& dev_type, const Options& opts, 
                              uint8_t busnum, uint8_t devnum)
{
  // FIXME: results must be libusb_unref_device()'ed
  libusb_device* dev = usb_find_device_by_path(busnum, devnum);

  if (!dev)
  {
    log_error << "USB device disappeared before it could be opened" << std::endl;
  }
  else
  {
    ControllerSlots::iterator it = m_controller_slots.end();
    for(ControllerSlots::iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
    {
      if (i->thread == 0)
      {
        it = i;
        break;
      }      
    }
    
    if (it == m_controller_slots.end())
    {
      log_error << "no free controller slot found, controller will be ignored" << std::endl;
    }
    else
    {
      std::auto_ptr<XboxGenericController> controller = XboxControllerFactory::create(dev_type, dev, opts);

      std::auto_ptr<MessageProcessor> message_proc;
      if (uinput)
      {
        message_proc.reset(new DefaultMessageProcessor(*uinput, it->config, opts));
      }
      else
      {
        message_proc.reset(new DummyMessageProcessor());
      }
      std::auto_ptr<XboxdrvThread> thread(new XboxdrvThread(message_proc, controller, opts));
      thread->start_thread(opts);
      it->thread = thread.release();

      log_info << "launched XboxdrvThread for " << boost::format("%03d:%03d")
        % static_cast<int>(busnum) 
        % static_cast<int>(devnum)
               << " in slot " << (it - m_controller_slots.begin())
               << ", free slots: " 
               << get_free_slot_count() << "/" << m_controller_slots.size()
               << std::endl;
    }
  }
}

int
XboxdrvDaemon::get_free_slot_count() const
{
  int slot_count = 0;

  for(ControllerSlots::const_iterator i = m_controller_slots.begin(); i != m_controller_slots.end(); ++i)
  {
    if (i->thread == 0)
    {
      slot_count += 1;
    }
  }

  return slot_count;
}

/* EOF */
