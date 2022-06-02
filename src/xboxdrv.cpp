/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include <fmt/format.h>
#include <errno.h>
#include <iostream>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <strut/join.hpp>
#include <strut/layout.hpp>
#include <unsebu/usb_gsource.hpp>
#include <unsebu/usb_helper.hpp>
#include <unsebu/usb_subsystem.hpp>

#include "command_line_options.hpp"
#include "controller/evdev_controller.hpp"
#include "controller_factory.hpp"
#include "controller_thread.hpp"
#include "evdev_helper.hpp"
#include "raise_exception.hpp"
#include "util/string.hpp"
#include "util/terminal.hpp"
#include "xboxdrv_daemon.hpp"
#include "xboxdrv_main.hpp"

namespace xboxdrv {

// Some ugly global variables, needed for sigint catching
bool global_exit_xboxdrv = false;

void
Xboxdrv::run_list_controller()
{
  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_init() failed: " << libusb_strerror(ret));
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
              fmt::print(" {:2d} |  {:2d} |   0x{:04x} |    0x{:04x} | {} (Port: {})\n",
                         id,
                         wid,
                         int(xpad_devices[i].idVendor),
                         int(xpad_devices[i].idProduct),
                         xpad_devices[i].name,
                         wid);
            }
          }
          else
          {
            fmt::print(" {:2d} |  {:2d} |   0x{:04x} |    0x{:04x} | {}\n",
                       id,
                       0,
                       int(xpad_devices[i].idVendor),
                       int(xpad_devices[i].idProduct),
                       xpad_devices[i].name);
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
    fmt::print("{} 0x{:04x} 0x{:04x} {}\n",
               gamepadtype_to_string(xpad_devices[i].type),
               int(xpad_devices[i].idVendor),
               int(xpad_devices[i].idProduct),
               xpad_devices[i].name);
  }
}

bool xpad_device_sorter(XPadDevice const& lhs, XPadDevice const& rhs)
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
  std::vector<XPadDevice> sorted_devices(xpad_devices_count);
  std::copy_n(xpad_devices, xpad_devices_count, sorted_devices.begin());
  std::sort(sorted_devices.begin(), sorted_devices.end(), xpad_device_sorter);

  for(int i = 0; i < xpad_devices_count; ++i)
  {

    std::string gamepadtype;
    switch(sorted_devices[i].type)
    {
      case GAMEPAD_XBOX:
        gamepadtype = "XTYPE_XBOX";
        break;

      case GAMEPAD_XBOX360:
        gamepadtype = "XTYPE_XBOX360";
        break;

      case GAMEPAD_XBOX360_WIRELESS:
        gamepadtype = "XTYPE_XBOX360W";
        break;

      case GAMEPAD_XBOXONE_WIRELESS:
        gamepadtype = "XTYPE_XBOXONE";
        break;

      default:
        continue;
    }

    fmt::print("{ 0x{:04x}, 0x{:04x}, \"{}\", 0, {} },\n",
               int(sorted_devices[i].idVendor),
               int(sorted_devices[i].idProduct),
               sorted_devices[i].name,
               gamepadtype);
  }
}

void
Xboxdrv::run_help_devices()
{
  std::cout << " idVendor | idProduct | Name" << std::endl;
  std::cout << "----------+-----------+---------------------------------" << std::endl;
  for(int i = 0; i < xpad_devices_count; ++i)
  {
    fmt::print("   0x{:04x} |    0x{:04x} | {}\n",
               int(xpad_devices[i].idVendor),
               int(xpad_devices[i].idProduct),
               xpad_devices[i].name);
  }
}

void
Xboxdrv::print_copyright() const
{
  strut::Layout wrap(std::cout, get_terminal_width());
  wrap.para("xboxdrv " PACKAGE_VERSION " - http://pingus.seul.org/~grumbel/xboxdrv/");
  wrap.para("Copyright © 2008-2012 Ingo Ruhnke <grumbel@gmail.com>");
  wrap.para("Licensed under GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>");
  wrap.para("This program comes with ABSOLUTELY NO WARRANTY.");
  wrap.para("This is free software, and you are welcome to redistribute it under certain "
            "conditions; see the file COPYING for details.");
  wrap.newline();
}

void
Xboxdrv::run_main(Options const& opts)
{
  if (!opts.quiet)
  {
    print_copyright();
  }

  unsebu::USBSubsystem usb_subsystem;
  XboxdrvMain xboxdrv_main(usb_subsystem, opts);
  xboxdrv_main.run();
}

void
Xboxdrv::run_daemon(Options const& opts)
{
  if (!opts.quiet)
  {
    print_copyright();
  }

  if (opts.usb_debug)
  {
    libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
  }

  if (!opts.detach)
  {
    unsebu::USBSubsystem usb_subsystem;
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
          unsebu::USBSubsystem usb_subsystem;
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

  strut::Layout wrap(std::cout, terminal_width);

  if (enums & Options::LIST_ABS)
  {
    wrap.println("EV_ABS:");
    wrap.para("  ", strut::join(evdev_abs_names.get_names(), ", "));
    wrap.newline();
  }

  if (enums & Options::LIST_REL)
  {
    wrap.println("EV_REL:");
    wrap.para("  ", strut::join(evdev_rel_names.get_names(), ", "));
    wrap.newline();
  }

  if (enums & Options::LIST_KEY)
  {
    wrap.println("EV_KEY:");
    wrap.para("  ", strut::join(evdev_key_names.get_names(), ", "));
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
    wrap.para("  ", strut::join(lst, ", "));
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
Xboxdrv::set_scheduling(Options const& opts)
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
  catch(std::exception const& err)
  {
    std::cout << "\n-- [ ERROR ] ------------------------------------------------------\n"
              << err.what() << std::endl;
  }

  return 0;
}

} // namespace xboxdrv

/* EOF */
