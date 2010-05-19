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

#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <sys/time.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

#include "uinput.hpp"
#include "xboxmsg.hpp"
#include "xbox_controller.hpp"
#include "xbox360_controller.hpp"
#include "xbox360_wireless_controller.hpp"
#include "firestorm_dual_controller.hpp"
#include "saitek_p2500_controller.hpp"
#include "helper.hpp"
#include "evdev_helper.hpp"
#include "command_line_options.hpp"
#include "xbox_generic_controller.hpp"

#include "xboxdrv.hpp"

// Some ugly global variables, needed for sigint catching
bool global_exit_xboxdrv = false;
XboxGenericController* global_controller = 0;

void on_sigint(int)
{
  if (global_exit_xboxdrv)
  {
    if (!command_line_options->quiet)
      std::cout << "Ctrl-c pressed twice, exiting hard" << std::endl;
    exit(EXIT_SUCCESS);
  }
  else
  {
    if (!command_line_options->quiet)
      std::cout << "Shutdown initiated, press Ctrl-c again if nothing is happening" << std::endl;

    global_exit_xboxdrv = true; 
    if (global_controller)
      global_controller->set_led(0);
  }
}

void on_sigterm(int)
{
  if (!command_line_options->quiet)
    std::cout << "Shutdown initiated by SIGTERM" << std::endl;

  if (global_controller)
    global_controller->set_led(0);

  exit(EXIT_SUCCESS);
}

void set_rumble(XboxGenericController* controller, int gain, uint8_t lhs, uint8_t rhs)
{
  lhs = std::min(lhs * gain / 255, 255);
  rhs = std::min(rhs * gain / 255, 255);
  
  //std::cout << (int)lhs << " " << (int)rhs << std::endl;

  controller->set_rumble(lhs, rhs);
}

void
Xboxdrv::run_list_controller()
{
  usb_init();
  usb_find_busses();
  usb_find_devices();

  struct usb_bus* busses = usb_get_busses();

  int id = 0;
  std::cout << " id | wid | idVendor | idProduct | Name" << std::endl;
  std::cout << "----+-----+----------+-----------+--------------------------------------" << std::endl;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
  {
    for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
    {
      for(int i = 0; i < xpad_devices_count; ++i)
      {
        if (dev->descriptor.idVendor  == xpad_devices[i].idVendor &&
            dev->descriptor.idProduct == xpad_devices[i].idProduct)
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
    std::cout << "\nNo controller detected" << std::endl; 
}

bool
Xboxdrv::find_controller_by_path(const char* busid, const char* devid,struct usb_device** xbox_device) const
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next)
  {
    if (strcmp(bus->dirname, busid) == 0)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
      {
        if (strcmp(dev->filename, devid) == 0)
        {
          *xbox_device = dev;
          return true;
        }
      }
    }
  }
  return 0;
}

/** find the number of the next unused /dev/input/jsX device */
int
Xboxdrv::find_jsdev_number() const
{
  for(int i = 0; ; ++i)
  {
    char filename1[32];
    char filename2[32];

    sprintf(filename1, "/dev/input/js%d", i);
    sprintf(filename2, "/dev/js%d", i);

    if (access(filename1, F_OK) != 0 && access(filename2, F_OK) != 0)
      return i;
  }
}

/** find the number of the next unused /dev/input/eventX device */
int
Xboxdrv::find_evdev_number() const
{
  for(int i = 0; ; ++i)
  {
    char filename[32];

    sprintf(filename, "/dev/input/event%d", i);

    if (access(filename, F_OK) != 0)
      return i;
  }
}

bool
Xboxdrv::find_controller_by_id(int id, int vendor_id, int product_id, struct usb_device** xbox_device) const
{
  struct usb_bus* busses = usb_get_busses();

  int id_count = 0;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
  {
    for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
    {
      if (dev->descriptor.idVendor  == vendor_id &&
          dev->descriptor.idProduct == product_id)
      {
        if (id_count == id)
        {
          *xbox_device = dev;
          return true;
        }
        else
        {
          id_count += 1;
          break;
        }
      }
    }
  }
  return 0;
  
}

bool
Xboxdrv::find_xbox360_controller(int id, struct usb_device** xbox_device, XPadDevice* type) const
{
  struct usb_bus* busses = usb_get_busses();

  int id_count = 0;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
  {
    for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
    {
      if (0)
        std::cout << (boost::format("UsbDevice: idVendor: 0x%04x idProduct: 0x%04x")
                      % int(dev->descriptor.idProduct)
                      % int(dev->descriptor.idVendor))
                  << std::endl;

      for(int i = 0; i < xpad_devices_count; ++i)
      {
        if (dev->descriptor.idVendor  == xpad_devices[i].idVendor &&
            dev->descriptor.idProduct == xpad_devices[i].idProduct)
        {
          if (id_count == id)
          {
            *xbox_device = dev;
            *type        = xpad_devices[i];
            return true;
          }
          else
          {
            id_count += 1;
            break;
          }
        }
      }
    }
  }
  return 0;
}

void
Xboxdrv::apply_modifier(XboxGenericMsg& msg, int msec_delta, const CommandLineOptions& opts) const
{
  apply_calibration_map(msg, opts.calibration_map);

  // Apply modifier
  apply_deadzone(msg, opts);

  if (opts.square_axis)
    apply_square_axis(msg);

  if (!opts.axis_sensitivity_map.empty())
    apply_axis_sensitivity(msg, opts);

  if (opts.four_way_restrictor)
    apply_four_way_restrictor(msg, opts);

  if (opts.dpad_rotation)
    apply_dpad_rotator(msg, opts);
}

void
Xboxdrv::controller_loop(GamepadType type, uInput* uinput, XboxGenericController* controller, const CommandLineOptions& opts)
{
  int timeout = 0; // 0 == no timeout
  XboxGenericMsg oldmsg; // last data send to uinput
  XboxGenericMsg oldrealmsg; // last data read from the device

  std::auto_ptr<AutoFireModifier>      autofire_modifier;
  std::auto_ptr<RelativeAxisModifier> relative_axis_modifier;

  if (!opts.autofire_map.empty())
    autofire_modifier.reset(new AutoFireModifier(opts.autofire_map)); 

  if (!opts.relative_axis_map.empty())
    relative_axis_modifier.reset(new RelativeAxisModifier(opts.relative_axis_map)); 

  // how long to wait for a controller event before taking care of autofire etc.
  timeout = 25; 

  memset(&oldmsg,     0, sizeof(oldmsg));
  memset(&oldrealmsg, 0, sizeof(oldrealmsg));

  uint32_t last_time = get_time();
  while(!global_exit_xboxdrv)
  {
    XboxGenericMsg msg;

    if (controller->read(msg, opts.verbose, timeout))
    {
      oldrealmsg = msg;
    }
    else
    {
      // no new data read, so copy the last read data
      msg = oldrealmsg;
    }

    // Calc changes in time
    uint32_t this_time = get_time();
    int msec_delta = this_time - last_time;
    last_time = this_time;

    apply_modifier(msg, msec_delta, opts);

    if (autofire_modifier.get())
      autofire_modifier->update(msec_delta, msg);
      
    if (relative_axis_modifier.get())
      relative_axis_modifier->update(msec_delta, msg);

    if (!opts.button_map.empty())
      apply_button_map(msg, opts.button_map);

    if (!opts.axis_map.empty())
      apply_axis_map(msg,   opts.axis_map);

    if (memcmp(&msg, &oldmsg, sizeof(XboxGenericMsg)) != 0)
    { // Only send a new event out if something has changed,
      // this is useful since some controllers send events
      // even if nothing has changed, deadzone can cause this
      // too
      oldmsg = msg;

      if (!opts.silent)
        std::cout << msg << std::endl;

      if (uinput) 
        uinput->send(msg);
                 
      if (opts.rumble)
      {
        if (type == GAMEPAD_XBOX)
        {
          set_rumble(controller, opts.rumble_gain, msg.xbox.lt, msg.xbox.rt);
        }
        else if (type == GAMEPAD_XBOX360 ||
                 type == GAMEPAD_XBOX360_WIRELESS)
        {
          set_rumble(controller, opts.rumble_gain, msg.xbox360.lt, msg.xbox360.rt);
        }
        else if (type == GAMEPAD_FIRESTORM ||
                 type == GAMEPAD_FIRESTORM_VSB)
        {
          set_rumble(controller, opts.rumble_gain,
                     std::min(255, abs((msg.xbox360.y1>>8)*2)), 
                     std::min(255, abs((msg.xbox360.y2>>8)*2)));
        }
      }
    }

    if (uinput)
      uinput->update(msec_delta);
  }
}

void
Xboxdrv::find_controller(struct usb_device*& dev,
                         XPadDevice&         dev_type,
                         const CommandLineOptions& opts) const
{
  if (opts.busid[0] != '\0' && opts.devid[0] != '\0')
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      std::cout << "Error: --device-by-path BUS:DEV option must be used in combination with --type TYPE option" << std::endl;
      exit(EXIT_FAILURE);
    }
    else
    {
      if (!find_controller_by_path(opts.busid, opts.devid, &dev))
      {
        std::cout << "Error: couldn't find device " << opts.busid << ":" << opts.devid << std::endl;
        exit(EXIT_FAILURE);
      }
      else
      {
        dev_type.type      = opts.gamepad_type;
        dev_type.idVendor  = dev->descriptor.idVendor;
        dev_type.idProduct = dev->descriptor.idProduct;
        dev_type.name      = "unknown";
      }
    }
  }
  else if (opts.vendor_id != -1 && opts.product_id != -1)
  {
    if (opts.gamepad_type == GAMEPAD_UNKNOWN)
    {
      std::cout << "Error: --device-by-id VENDOR:PRODUCT option must be used in combination with --type TYPE option" << std::endl;
      exit(EXIT_FAILURE);
    }
    else 
    {
      if (!find_controller_by_id(opts.controller_id, opts.vendor_id, opts.product_id, &dev))
      {
        std::cout << "Error: couldn't find device with " 
                  << (boost::format("%04x:%04x") % opts.vendor_id % opts.product_id) 
                  << std::endl;
        exit(EXIT_FAILURE);
      }
      else
      {
        dev_type.type = opts.gamepad_type;
        dev_type.idVendor  = opts.vendor_id;
        dev_type.idProduct = opts.product_id;
        dev_type.name = "unknown";
      }
    }
  }
  else
  {
    if (!find_xbox360_controller(opts.controller_id, &dev, &dev_type))
    {
      std::cout << "No Xbox or Xbox360 controller found" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void
Xboxdrv::run_main(const CommandLineOptions& opts)
{
  if (!opts.quiet)
  {
    opts.print_version();
    std::cout << std::endl;
  }

  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  struct usb_device* dev      = 0;
  XPadDevice         dev_type;
  
  find_controller(dev, dev_type, opts);

  if (!dev)
  {
    std::cout << "No suitable USB device found, abort" << std::endl;
    exit(EXIT_FAILURE);
  }
  else 
  {
    if (!opts.quiet)
      print_info(dev, dev_type, opts);

    std::auto_ptr<XboxGenericController> controller;

    switch (dev_type.type)
    {
      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        controller = std::auto_ptr<XboxGenericController>(new XboxController(dev));
        break;

      case GAMEPAD_XBOX360_GUITAR:
        controller = std::auto_ptr<XboxGenericController>(new Xbox360Controller(dev, true));
        break;

      case GAMEPAD_XBOX360:
        controller = std::auto_ptr<XboxGenericController>(new Xbox360Controller(dev, false));
        break;

      case GAMEPAD_XBOX360_WIRELESS:
        controller = std::auto_ptr<XboxGenericController>(new Xbox360WirelessController(dev, opts.wireless_id));
        break;

      case GAMEPAD_FIRESTORM:
        controller = std::auto_ptr<XboxGenericController>(new FirestormDualController(dev, false));
        break;

      case GAMEPAD_FIRESTORM_VSB:
        controller = std::auto_ptr<XboxGenericController>(new FirestormDualController(dev, true));
        break;

      case GAMEPAD_SAITEK_P2500:
        controller = std::auto_ptr<XboxGenericController>(new SaitekP2500Controller(dev));
        break;

      default:
        assert(!"Unknown gamepad type");
    }

    global_controller = controller.get();

    int jsdev_number = find_jsdev_number();
    int evdev_number = find_evdev_number();

    if (opts.led == -1)
      controller->set_led(2 + jsdev_number % 4);
    else
      controller->set_led(opts.led);

    if (opts.rumble_l != -1 && opts.rumble_r != -1)
    { // Only set rumble when explicitly requested
      controller->set_rumble(opts.rumble_l, opts.rumble_r);
    }
      
    if (opts.instant_exit)
    {
      usleep(1000);
    }
    else
    {          
      std::auto_ptr<uInput> uinput;
      if (!opts.no_uinput)
      {
        if (!opts.quiet)
          std::cout << "\nStarting with uinput... " << std::flush;
        uinput = std::auto_ptr<uInput>(new uInput(dev_type, opts.uinput_config));
        uinput->set_ff_callback(boost::bind(&set_rumble,  controller.get(), opts.rumble_gain, _1, _2));
        if (!opts.quiet)
          std::cout << "done" << std::endl;
      }
      else
      {
        if (!opts.quiet)
          std::cout << "Starting without uinput" << std::endl;
      }

      if (!opts.quiet)
      {
        std::cout << "\nYour Xbox/Xbox360 controller should now be available as:" << std::endl
                  << "  /dev/input/js" << jsdev_number << std::endl
                  << "  /dev/input/event" << evdev_number << std::endl;
          
        std::cout << "\nPress Ctrl-c to quit\n" << std::endl;
      }

      global_exit_xboxdrv = false;
      controller_loop(dev_type.type, uinput.get(), controller.get(), opts);
          
      if (!opts.quiet) 
        std::cout << "Shutdown complete" << std::endl;
    }
  }
}

void
Xboxdrv::print_info(struct usb_device* dev,
                    const XPadDevice& dev_type,
                    const CommandLineOptions& opts) const
{
  std::cout << "USB Device:        " << dev->bus->dirname << ":" << dev->filename << std::endl;
  std::cout << "Controller:        " << boost::format("\"%s\" (idVendor: 0x%04x, idProduct: 0x%04x)")
    % dev_type.name % uint16_t(dev->descriptor.idVendor) % uint16_t(dev->descriptor.idProduct) << std::endl;
  if (dev_type.type == GAMEPAD_XBOX360_WIRELESS)
    std::cout << "Wireless Port:     " << opts.wireless_id << std::endl;
  std::cout << "Controller Type:   " << dev_type.type << std::endl;
  std::cout << "Deadzone:          " << opts.deadzone << std::endl;
  std::cout << "Trigger Deadzone:  " << opts.deadzone_trigger << std::endl;
  std::cout << "Rumble Debug:      " << (opts.rumble ? "on" : "off") << std::endl;
  std::cout << "Rumble Speed:      " << "left: " << opts.rumble_l << " right: " << opts.rumble_r << std::endl;
  if (opts.led == -1)
    std::cout << "LED Status:        " << "auto" << std::endl;
  else
    std::cout << "LED Status:        " << opts.led << std::endl;

  std::cout << "Square Axis:       " << ((opts.square_axis) ? "yes" : "no") << std::endl;
  std::cout << "4-Way Restrictor:  " << ((opts.four_way_restrictor) ? "yes" : "no") << std::endl;
  std::cout << "Dpad Rotation:     " << opts.dpad_rotation * 45 << " degree" << std::endl;
  
  std::cout << "ButtonMap:         ";
  if (opts.button_map.empty())
  {
    std::cout << "none" << std::endl;
  }
  else
  {
    for(std::vector<ButtonMapping>::const_iterator i = opts.button_map.begin(); i != opts.button_map.end(); ++i)
    {
      std::cout << btn2string(i->lhs) << "->" << btn2string(i->rhs) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "AxisMap:           ";
  if (opts.axis_map.empty())
  {
    std::cout << "none" << std::endl;
  }
  else
  {
    for(std::vector<AxisMapping>::const_iterator i = opts.axis_map.begin(); i != opts.axis_map.end(); ++i)
    {
      if (i->invert)
        std::cout << "-" << axis2string(i->lhs) << "->" << axis2string(i->rhs) << " ";
      else
        std::cout << axis2string(i->lhs) << "->" << axis2string(i->rhs) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "RelativeAxisMap:   ";
  if (opts.relative_axis_map.empty())
  {
    std::cout << "none" << std::endl;
  }
  else
  {
    for(std::vector<RelativeAxisMapping>::const_iterator i = opts.relative_axis_map.begin(); i != opts.relative_axis_map.end(); ++i)
    {
      std::cout << axis2string(i->axis) << "=" << i->speed << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "AutoFireMap:       ";
  if (opts.autofire_map.empty())
  {
    std::cout << "none" << std::endl;
  }
  else
  {
    for(std::vector<AutoFireMapping>::const_iterator i = opts.autofire_map.begin(); i != opts.autofire_map.end(); ++i)
    {
      std::cout << btn2string(i->button) << "=" << i->frequency << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "RumbleGain:        " << opts.rumble_gain << std::endl;
  std::cout << "ForceFeedback:     " << ((opts.uinput_config.force_feedback) ? "enabled" : "disabled") << std::endl;
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
Xboxdrv::run_daemon(const CommandLineOptions& opts)
{
  pid_t pid = fork();

  if (pid < 0) exit(EXIT_FAILURE); /* fork error */
  if (pid > 0) exit(EXIT_SUCCESS); /* parent exits */

  pid_t sid = setsid();
  std::cout << "Sid: " << sid << std::endl;
  if (chdir("/") != 0)
  {
    throw std::runtime_error(strerror(errno));
  }

  run_main(opts);
}

int
Xboxdrv::main(int argc, char** argv)
{
  try 
  {
    signal(SIGINT,  on_sigint);
    signal(SIGTERM, on_sigterm);

    CommandLineOptions opts;
    opts.parse_args(argc, argv);
    command_line_options = &opts;

    switch(opts.mode)
    {
      case CommandLineOptions::PRINT_HELP_DEVICES:
        run_help_devices();
        break;

      case CommandLineOptions::RUN_LIST_SUPPORTED_DEVICES:
        run_list_supported_devices();
        break;

      case CommandLineOptions::RUN_LIST_SUPPORTED_DEVICES_XPAD:
        run_list_supported_devices_xpad();
        break;

      case CommandLineOptions::PRINT_VERSION:
        opts.print_version();
        break;

      case CommandLineOptions::PRINT_HELP:
        opts.print_help();
        break;

      case CommandLineOptions::PRINT_LED_HELP:
        opts.print_led_help();
        break;

      case CommandLineOptions::RUN_DEFAULT:
        run_main(opts);
        break;

      case CommandLineOptions::RUN_DAEMON:
        run_daemon(opts);
        break;

      case CommandLineOptions::RUN_LIST_CONTROLLER:
        run_list_controller();
        break;
    }
  }
  catch(std::exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
  }

  return 0;
}

int main(int argc, char** argv)
{
  Xboxdrv xboxdrv;
  return xboxdrv.main(argc, argv);
}

/* EOF */
