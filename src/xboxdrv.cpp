/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
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

#include "xboxdrv.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>
#include <errno.h>
#include <iostream>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "command_line_options.hpp"
#include "controller/evdev_controller.hpp"
#include "controller_factory.hpp"
#include "controller_thread.hpp"
#include "evdev_helper.hpp"
#include "helper.hpp"
#include "raise_exception.hpp"
#include "usb_gsource.hpp"
#include "usb_helper.hpp"
#include "usb_subsystem.hpp"
#include "word_wrap.hpp"
#include "xboxdrv_daemon.hpp"
#include "xboxdrv_main.hpp"

// Some ugly global variables, needed for sigint catching
bool global_exit_xboxdrv = false;

void
Xboxdrv::run_list_controller()
{
  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_init() failed: " << usb_strerror(ret));
  }

  libusb_device** list;
  ssize_t num_devices = libusb_get_device_list(NULL, &list);

  int id = 0;
  std::cout << " id | wid | idVendor | idProduct | Name" << std::endl;
  std::cout << "----+-----+----------+-----------+--------------------------------------" << std::endl;

  for(ssize_t dev_it = 0; dev_it < num_devices; ++dev_it)
  {
    libusb_device* dev = list[dev_it];
    libusb_device_descriptor desc;

    // FIXME: we silently ignore failures
    if (libusb_get_device_descriptor(dev, &desc) == LIBUSB_SUCCESS)
    {
      for(int i = 0; i < xpad_devices_count; ++i)
      {
        if (desc.idVendor  == xpad_devices[i].idVendor &&
            desc.idProduct == xpad_devices[i].idProduct)
        {
          if (xpad_devices[i].type == GAMEPAD_XBOX360_WIRELESS)
          {
            for(int wid = 0; wid < 4; ++wid)
            {
              std::cout << boost::format(" %2d |  %2d |   0x%04x |    0x%04x | %s (Port: %s)")
                % id
                % wid
                % int(xpad_devices[i].idVendor)
                % int(xpad_devices[i].idProduct)
                % xpad_devices[i].name 
                % wid
                        << std::endl;
            }
          }
          else
          {
            std::cout << boost::format(" %2d |  %2d |   0x%04x |    0x%04x | %s")
              % id
              % 0
              % int(xpad_devices[i].idVendor)
              % int(xpad_devices[i].idProduct)
              % xpad_devices[i].name 
                      << std::endl;
          }
          id += 1;
          break;
        }
      }
    }
  }

  if (id == 0)
    std::cout << "\nno controller detected" << std::endl; 

  libusb_free_device_list(list, 1 /* unref_devices */);
}

void
Xboxdrv::run_list_supported_devices()
{
  for(int i = 0; i < xpad_devices_count; ++i)
  {
    std::cout << boost::format("%s 0x%04x 0x%04x %s\n")
      % gamepadtype_to_string(xpad_devices[i].type)
      % int(xpad_devices[i].idVendor)
      % int(xpad_devices[i].idProduct)
      % xpad_devices[i].name;
  }    
}

bool xpad_device_sorter(const XPadDevice& lhs, const XPadDevice& rhs)
{
  if (lhs.idVendor < rhs.idVendor)
  {
    return true;
  }
  else if (lhs.idVendor == rhs.idVendor)
  {
    return lhs.idProduct < rhs.idProduct;
  }
  else
  {
    return false;
  }
}

void
Xboxdrv::run_list_supported_devices_xpad()
{
  boost::scoped_array<XPadDevice> sorted_devices(new XPadDevice[xpad_devices_count]);
  memcpy(sorted_devices.get(), xpad_devices, sizeof(XPadDevice) * xpad_devices_count);

  std::sort(sorted_devices.get(), sorted_devices.get() + xpad_devices_count, xpad_device_sorter);

  for(int i = 0; i < xpad_devices_count; ++i)
  {
    std::cout << boost::format("{ 0x%04x, 0x%04x, \"%s\", %s },\n")
      % int(sorted_devices[i].idVendor)
      % int(sorted_devices[i].idProduct)
      % sorted_devices[i].name
      % gamepadtype_to_macro_string(sorted_devices[i].type);
  }    
}

void
Xboxdrv::run_help_devices()
{
  std::cout << " idVendor | idProduct | Name" << std::endl;
  std::cout << "----------+-----------+---------------------------------" << std::endl;
  for(int i = 0; i < xpad_devices_count; ++i)
  {
    std::cout << boost::format("   0x%04x |    0x%04x | %s")
      % int(xpad_devices[i].idVendor)
      % int(xpad_devices[i].idProduct)
      % xpad_devices[i].name 
              << std::endl;
  }           
}

void
Xboxdrv::print_copyright() const
{
  WordWrap wrap(get_terminal_width());
  wrap.para("xboxdrv " PACKAGE_VERSION " - http://pingus.seul.org/~grumbel/xboxdrv/");
  wrap.para("Copyright © 2008-2012 Ingo Ruhnke <grumbel@gmx.de>");
  wrap.para("Licensed under GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>");
  wrap.para("This program comes with ABSOLUTELY NO WARRANTY.");
  wrap.para("This is free software, and you are welcome to redistribute it under certain "
            "conditions; see the file COPYING for details.");
  wrap.newline();
}

void
Xboxdrv::run_main(const Options& opts)
{
  if (!opts.quiet)
  {
    print_copyright();
  }

  USBSubsystem usb_subsystem;
  XboxdrvMain xboxdrv_main(usb_subsystem, opts);
  xboxdrv_main.run();
}

void
Xboxdrv::run_daemon(const Options& opts)
{
  if (!opts.quiet)
  {
    print_copyright();
  }

  if (opts.usb_debug)
  {
    libusb_set_debug(NULL, 3);
  }

  USBSubsystem usb_subsystem;
  if (!opts.detach)
  {
    XboxdrvDaemon daemon(usb_subsystem, opts);
    daemon.run();
  }
  else
  {
    pid_t pid = fork();

    if (pid < 0) 
    { // fork error
      raise_exception(std::runtime_error, "failed to fork(): " << strerror(errno));
    }
    else if (pid > 0) 
    { // parent, just exit
      _exit(EXIT_SUCCESS);
    }
    else
    { // child, run daemon
      pid_t sid = setsid();

      if (sid == static_cast<pid_t>(-1))
      {
        raise_exception(std::runtime_error, "failed to setsid(): " << strerror(errno));
      }
      else
      {
        if (chdir("/") != 0)
        {
          raise_exception(std::runtime_error, "failed to chdir(\"/\"): " << strerror(errno));
        }
        else
        {
          XboxdrvDaemon daemon(usb_subsystem, opts);
          daemon.run();
        }
      }
    }
  }
}

void
Xboxdrv::run_list_enums(uint32_t enums)
{
  const int terminal_width = get_terminal_width();

  WordWrap wrap(terminal_width);

  if (enums & Options::LIST_ABS)
  {
    wrap.println("EV_ABS:");
    wrap.para("  ", boost::algorithm::join(evdev_abs_names.get_names(), ", "));
    wrap.newline();
  }
  
  if (enums & Options::LIST_REL)
  {
    wrap.println("EV_REL:");
    wrap.para("  ", boost::algorithm::join(evdev_rel_names.get_names(), ", "));
    wrap.newline();
  }
  
  if (enums & Options::LIST_KEY)
  {
    wrap.println("EV_KEY:");
    wrap.para("  ", boost::algorithm::join(evdev_key_names.get_names(), ", "));
    wrap.newline();
  }
  
  if (enums & Options::LIST_X11KEYSYM)
  {
    std::vector<std::string> lst;  
    for(X11KeysymEnum::const_iterator i = get_x11keysym_names().begin();
        i != get_x11keysym_names().end(); ++i)
    {
      lst.push_back(i->second);
    }
    wrap.println("X11Keysym:");
    wrap.para("  ", boost::algorithm::join(lst, ", "));
    wrap.newline();
  }
}

Xboxdrv::Xboxdrv()
{
}

Xboxdrv::~Xboxdrv()
{
}

void
Xboxdrv::set_scheduling(const Options& opts)
{
  if (opts.priority == Options::kPriorityRealtime)
  {
    // try to set realtime priority when root, as user there doesn't
    // seem to be a way to increase the priority
    log_info("enabling realtime priority scheduling");

    int policy = SCHED_RR;
    
    struct sched_param param;
    memset(&param, 0, sizeof(struct sched_param));
    param.sched_priority = sched_get_priority_max(policy);

    // we don't try SCHED_OTHER for users as min and max priority is
    // 0 for that, thus we can't change anything with that

    int ret;
    if ((ret = sched_setscheduler(getpid(), policy, &param)) != 0)
    {          
      raise_exception(std::runtime_error, "sched_setschedparam() failed: " << strerror(errno));
    }
  }
}

int
Xboxdrv::main(int argc, char** argv)
{
  try 
  {
    Options opts;

    CommandLineParser cmd_parser;
    cmd_parser.parse_args(argc, argv, &opts);

    set_scheduling(opts);

    switch(opts.mode)
    {
      case Options::PRINT_HELP_DEVICES:
        run_help_devices();
        break;

      case Options::RUN_LIST_SUPPORTED_DEVICES:
        run_list_supported_devices();
        break;

      case Options::RUN_LIST_SUPPORTED_DEVICES_XPAD:
        run_list_supported_devices_xpad();
        break;

      case Options::PRINT_VERSION:
        cmd_parser.print_version();
        break;

      case Options::PRINT_HELP:
        cmd_parser.print_help();
        break;

      case Options::PRINT_LED_HELP:
        cmd_parser.print_led_help();
        break;

      case Options::RUN_DEFAULT:
        run_main(opts);
        break;

      case Options::PRINT_ENUMS:
        run_list_enums(opts.list_enums);
        break;

      case Options::RUN_DAEMON:
        run_daemon(opts);
        break;

      case Options::RUN_LIST_CONTROLLER:
        run_list_controller();
        break;
    }
  }
  catch(const std::exception& err)
  {
    std::cout << "\n-- [ ERROR ] ------------------------------------------------------\n"
              << err.what() << std::endl;
  }

  return 0;
}

/* EOF */
