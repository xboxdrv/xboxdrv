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
#include "uinput.hpp"
#include "xboxmsg.hpp"
#include "xbox_controller.hpp"
#include "xbox360_controller.hpp"
#include "xbox360_wireless_controller.hpp"
#include "firestorm_dual_controller.hpp"
#include "helper.hpp"
#include "evdev_helper.hpp"
#include "command_line_options.hpp"
#include "xbox_generic_controller.hpp"

#include "xboxdrv.hpp"

// Some ugly global variables, needed for sigint catching
bool global_exit_xboxdrv = false;
XboxGenericController* global_controller = 0;

// FIXME: We shouldn't check device-ids, but device class or so, to
// automatically catch all third party stuff
XPadDevice xpad_devices[] = {
  // Evil?! Anymore info we could use to identify the devices?
  // { GAMEPAD_XBOX,             0x0000, 0x0000, "Generic X-Box pad" },
  // { GAMEPAD_XBOX,             0xffff, 0xffff, "Chinese-made Xbox Controller" },

  // These should work
  { GAMEPAD_XBOX,             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0287, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft X-Box pad v2 (US)" }, // duplicate
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft Xbox Controller S" }, // duplicate
  { GAMEPAD_XBOX,             0x046d, 0xca84, "Logitech Xbox Cordless Controller" },
  { GAMEPAD_XBOX,             0x046d, 0xca88, "Logitech Compact Controller for Xbox" },
  { GAMEPAD_XBOX,             0x05fd, 0x1007, "Mad Catz Controller (unverified)" },
  { GAMEPAD_XBOX,             0x05fd, 0x107a, "InterAct 'PowerPad Pro' X-Box pad (Germany)" },
  { GAMEPAD_XBOX,             0x0738, 0x4516, "Mad Catz Control Pad" },
  { GAMEPAD_XBOX,             0x0738, 0x4522, "Mad Catz LumiCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4526, "Mad Catz Control Pad Pro" },
  { GAMEPAD_XBOX,             0x0738, 0x4536, "Mad Catz MicroCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4556, "Mad Catz Lynx Wireless Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8802, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8810, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x9902, "HAMA VibraX - *FAULTY HARDWARE*" },
  { GAMEPAD_XBOX,             0x0e4c, 0x1097, "Radica Gamester Controller" },
  { GAMEPAD_XBOX,             0x0e4c, 0x2390, "Radica Games Jtech Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0003, "Logic3 Freebird wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0005, "Eclipse wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0006, "Edge wireless Controller" },
  { GAMEPAD_XBOX,             0x0e8f, 0x0201, "SmartJoy Frag Xpad/PS2 adaptor" },
  { GAMEPAD_XBOX,             0x0f30, 0x0202, "Joytech Advanced Controller" },
  { GAMEPAD_XBOX,             0x0f30, 0x8888, "BigBen XBMiniPad Controller" },
  { GAMEPAD_XBOX,             0x102c, 0xff0c, "Joytech Wireless Advanced Controller" },
  { GAMEPAD_XBOX,             0x044f, 0x0f07, "Thrustmaster, Inc. Controller" },
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4726, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0f0d, 0x000d, "Hori Fighting Stick Ex2" },
  { GAMEPAD_XBOX360,          0x162e, 0xbeef, "Joytech Neo-Se Take2" },
  { GAMEPAD_XBOX360,          0x046d, 0xc242, "Logitech ChillStream" },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0002, "Harmonix Guitar for Xbox 360" },
  { GAMEPAD_XBOX360_GUITAR,   0x1bad, 0x0003, "Harmonix Drum Kit for Xbox 360" },

  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" },

  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR dancepad" },
  { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" },

  { GAMEPAD_XBOX360,          0x12ab, 0x0004, "DDR Universe 2 Mat" }, 

  { GAMEPAD_FIRESTORM,        0x044f, 0xb304, "ThrustMaster, Inc. Firestorm Dual Power" },
};

const int xpad_devices_count = sizeof(xpad_devices)/sizeof(XPadDevice);

void arg2apply(const std::string& str, const boost::function<void (const std::string&)>& func)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == ',')
        {
          if (i != start)
            func(std::string(start, i));
          
          start = i+1;
        }
    }
  
  if (start != str.end())
    func(std::string(start, str.end()));
}

template<class C, class Func>
void arg2vector(const std::string& str, typename std::vector<C>& lst, Func func)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == ',')
        {
          if (i != start)
            lst.push_back(func(std::string(start, i)));
          
          start = i+1;
        }
    }
  
  if (start != str.end())
    lst.push_back(func(std::string(start, str.end())));
}

bool is_number(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    if (!isdigit(*i))
      return false;
  return true;
}

void set_ui_button_map(ButtonEvent* ui_button_map, const std::string& str)
{
  std::string::size_type i = str.find_first_of('=');
  if (i == std::string::npos)
    {
      throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-button-mapping, '=' missing");
    }
  else
    {
      //std::cout << string2btn(str.substr(0, i)) << " -> " << str.substr(i+1, str.size()-i) << std::endl;

      XboxButton  btn   = string2btn(str.substr(0, i));
      ButtonEvent event = ButtonEvent::from_string(str.substr(i+1, str.size()-i));
      
      if (btn != XBOX_BTN_UNKNOWN)
        {
          ui_button_map[btn] = event;
        }
      else
        {
          throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-button-mapping, Xbox button name not valid");
        }
    }
}

void set_ui_axis_map(AxisEvent* ui_axis_map, const std::string& str)
{
  std::string::size_type i = str.find_first_of('=');
  if (i == std::string::npos)
    {
      throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-axis-mapping");
    }
  else
    {
      XboxAxis  axis  = string2axis(str.substr(0, i));
      AxisEvent event = AxisEvent::from_string(str.substr(i+1, str.size()-i));
            
      if (axis != XBOX_AXIS_UNKNOWN)
        {
          ui_axis_map[axis] = event;
        }
      else
        {
          throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-button-mapping");
        }      
    }  
}

void list_controller()
{
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

bool find_controller_by_path(const char* busid, const char* devid,struct usb_device** xbox_device)
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
int find_jsdev_number()
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
int find_evdev_number()
{
  for(int i = 0; ; ++i)
    {
      char filename[32];

      sprintf(filename, "/dev/input/event%d", i);

      if (access(filename, F_OK) != 0)
        return i;
    }
}

bool find_controller_by_id(int id, int vendor_id, int product_id, struct usb_device** xbox_device)
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

bool find_xbox360_controller(int id, struct usb_device** xbox_device, XPadDevice* type)
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

void print_command_line_help(int argc, char** argv)
{
  std::cout << "Usage: " << argv[0] << " [OPTION]..." << std::endl;
  std::cout << "Xbox360 USB Gamepad Userspace Driver" << std::endl;
  std::cout << std::endl;
  std::cout << "General Options: " << std::endl;
  std::cout << "  -h, --help               display this help and exit" << std::endl;
  std::cout << "  -V, --version            print the version number and exit" << std::endl;
  std::cout << "  -v, --verbose            print verbose messages" << std::endl;
  std::cout << "  --help-led               list possible values for the led" << std::endl;
  std::cout << "  --help-devices           list supported devices" << std::endl;
  std::cout << "  -s, --silent             do not display events on console" << std::endl;
  std::cout << "  --quiet                  do not display startup text" << std::endl;
  std::cout << "  -i, --id N               use controller with id N (default: 0)" << std::endl;
  std::cout << "  -w, --wid N              use wireless controller with wid N (default: 0)" << std::endl;
  std::cout << "  -L, --list-controller    list available controllers" << std::endl;
  std::cout << "  -R, --test-rumble        map rumbling to LT and RT (for testing only)" << std::endl;
  std::cout << "  --no-uinput              do not try to start uinput event dispatching" << std::endl;
  std::cout << "  --mimic-xpad             Causes xboxdrv to use the same axis and button names as the xpad kernel driver" << std::endl;
  std::cout << "  -D, --daemon             run as daemon" << std::endl;
  std::cout << std::endl;
  std::cout << "Device Options: " << std::endl;
  std::cout << "  --device-by-path BUS:DEV\n"
            << "                           Use device BUS:DEV, do not do any scanning" << std::endl;
  std::cout << "  --device-by-id VENDOR:PRODUCT\n"
            << "                           Use device that matches VENDOR:PRODUCT (as returned by lsusb)" << std::endl;
  std::cout << std::endl;
  std::cout << "Status Options: " << std::endl;
  std::cout << "  -l, --led NUM            set LED status, see --list-led-values (default: 0)" << std::endl;
  std::cout << "  -r, --rumble L,R         set the speed for both rumble motors [0-255] (default: 0,0)" << std::endl;
  std::cout << "  -q, --quit               only set led and rumble status then quit" << std::endl;
  std::cout << std::endl;
  std::cout << "Configuration Options: " << std::endl;
  std::cout << "  --deadzone INT           Threshold under which axis events are ignored (default: 0)" << std::endl;
  std::cout << "  --trigger-as-button      LT and RT send button instead of axis events" << std::endl;
  std::cout << "  --trigger-as-zaxis       Combine LT and RT to form a zaxis instead" << std::endl;
  std::cout << "  --dpad-as-button         DPad sends button instead of axis events" << std::endl;
  std::cout << "  --dpad-only              Both sticks are ignored, only DPad sends out axis events" << std::endl;
  std::cout << "  --type TYPE              Ignore autodetection and enforce controller type\n"
            << "                           (xbox, xbox-mat, xbox360, xbox360-wireless, xbox360-guitar)" << std::endl;
  std::cout << "  -b, --buttonmap MAP      Remap the buttons as specified by MAP (example: B=A,X=A,Y=A)" << std::endl;
  std::cout << "  -a, --axismap MAP        Remap the axis as specified by MAP (example: -Y1=Y1,X1=X2)" << std::endl;

  std::cout << "  --name DEVNAME           Changes the descriptive name the device will have" << std::endl;
  std::cout << "  --ui-clear               Removes all existing uinput bindings" << std::endl;
  std::cout << "  --ui-buttonmap MAP       Changes the uinput events send when hitting a button (example: X=BTN_Y,A=KEY_A)" << std::endl;
  std::cout << "  --ui-axismap MAP         Changes the uinput events send when moving a axis (example: X1=ABS_X2)" << std::endl;

  std::cout << "  --square-axis            Cause the diagonals to be reported as (1,1) instead of (0.7, 0.7)" << std::endl;
  std::cout << "  --relative-axis MAP      Make an axis emulate a joystick throttle (example: y2=64000)" << std::endl;
  std::cout << "  --autofire MAP           Cause the given buttons to act as autofire (example: A=250)" << std::endl;
  std::cout << "  --calibration MAP        Cause the given buttons to act as autofire (example: X2=-32768:0:32767)" << std::endl;
  std::cout << "  --force-feedback         Enable force feedback support" << std::endl;
  std::cout << "  --rumble-gain NUM        Set relative rumble strength (default: 255)" << std::endl;
  std::cout << std::endl;
  std::cout << "See README for more documentation and examples." << std::endl;
  std::cout << "Report bugs to Ingo Ruhnke <grumbel@gmx.de>" << std::endl;
}

void print_led_help()
{
  std::cout << 
    "Possible values for '--led VALUE' are:\n\n"
    "   0: off\n"
    "   1: all blinking\n"
    "   2: 1/top-left blink, then on\n"
    "   3: 2/top-right blink, then on\n"
    "   4: 3/bottom-left blink, then on\n"
    "   5: 4/bottom-right blink, then on\n"
    "   6: 1/top-left on\n"
    "   7: 2/top-right on\n"
    "   8: 3/bottom-left on\n"
    "   9: 4/bottom-right on\n"
    "  10: rotate\n"
    "  11: blink\n"
    "  12: blink slower\n"
    "  13: rotate with two lights\n"
    "  14: blink\n"
    "  15: blink once\n"
            << std::endl;
}

void print_version()
{
  std::cout
    << "xboxdrv 0.4.6\n"
    << "Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>\n"
    << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
    << "This is free software: you are free to change and redistribute it.\n"
    << "There is NO WARRANTY, to the extent permitted by law."
    << std::endl;
}

int to_number(int range, const std::string& str)
{
  if (str.empty())
    {
      return 0;
    }
  else
    {
      if (str[str.size() - 1] == '%')
        {
          int percent = boost::lexical_cast<int>(str.substr(0, str.size()-1));
          return range * percent / 100;
        }
      else
        {
          return boost::lexical_cast<int>(str);
        }
    }
}

void parse_command_line(int argc, char** argv, CommandLineOptions& opts)
{  
  for(int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-h") == 0 ||
          strcmp(argv[i], "--help") == 0)
        {
          print_command_line_help(argc, argv);
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "-v") == 0 ||
          strcmp(argv[i], "--verbose") == 0)
        {
          opts.verbose = true;
        }
      else if (strcmp(argv[i], "-V") == 0 ||
          strcmp(argv[i], "--version") == 0)
        {
          print_version();
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "--quiet") == 0)
        {
          opts.quiet   = true;
        }
      else if (strcmp(argv[i], "-s") == 0 ||
               strcmp(argv[i], "--silent") == 0)
        {
          opts.silent = true;
        }
      else if (strcmp(argv[i], "--daemon") == 0 ||
               strcmp(argv[i], "-D") == 0)
        {
          opts.silent = true;
          opts.daemon = true;
        }
      else if (strcmp(argv[i], "--test-rumble") == 0 ||
               strcmp(argv[i], "-R") == 0)
        {
          opts.rumble = true;
        }
      else if (strcmp(argv[i], "-r") == 0 ||
               strcmp(argv[i], "--rumble") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (sscanf(argv[i], "%d,%d", &opts.rumble_l, &opts.rumble_r) == 2)
                {
                  opts.rumble_l = std::max(0, std::min(255, opts.rumble_l));
                  opts.rumble_r = std::max(0, std::min(255, opts.rumble_r));
                }
              else
                {
                  std::cout << "Error: " << argv[i-1] << " expected an argument in form INT,INT" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "-q") == 0 ||
               strcmp(argv[i], "--quit") == 0)
        {
          opts.instant_exit = true;
        }
      else if (strcmp(argv[i], "--no-uinput") == 0)
        {
          opts.no_uinput = true;
        }
      else if (strcmp(argv[i], "--mimic-xpad") == 0)
        {
          //opts.mimic_xpad = true;
        }
      else if (strcmp(argv[i], "-t") == 0 ||
               strcmp(argv[i], "--type") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (strcmp(argv[i], "xbox") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX;
                }
              else if (strcmp(argv[i], "xbox-mat") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX_MAT;
                }
              else if (strcmp(argv[i], "xbox360") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360;
                }
              else if (strcmp(argv[i], "xbox360-guitar") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360_GUITAR;
                }
              else if (strcmp(argv[i], "xbox360-wireless") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360_WIRELESS;
                }
              else if (strcmp(argv[i], "firestorm") == 0)
                {
                  opts.gamepad_type = GAMEPAD_FIRESTORM;
                }
              else
                {
                  std::cout << "Error: unknown type: " << argv[i] << std::endl;
                  std::cout << "Possible types are:" << std::endl;
                  std::cout << " * xbox" << std::endl;
                  std::cout << " * xbox-mat" << std::endl;
                  std::cout << " * xbox360" << std::endl;
                  std::cout << " * xbox360-guitar" << std::endl;
                  std::cout << " * xbox360-wireless" << std::endl;
                  std::cout << " * firestorm" << std::endl;
                  exit(EXIT_FAILURE); 
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "--force-feedback") == 0)
        {
          opts.uinput_config.force_feedback = true;
        }
      else if (strcmp(argv[i], "--rumble-gain") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.rumble_gain = to_number(255, argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-b") == 0 ||
               strcmp(argv[i], "--buttonmap") == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2vector(argv[i], opts.button_map, &ButtonMapping::from_string);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-a") == 0 ||
               strcmp(argv[i], "--axismap") == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2vector(argv[i], opts.axis_map, &AxisMapping::from_string);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "--name") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.uinput_config.device_name = argv[i];
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }                  
        }
      else if (strcmp(argv[i], "--ui-clear") == 0)
        {
          std::fill_n(opts.uinput_config.axis_map, (int)XBOX_AXIS_MAX, AxisEvent::invalid());
          std::fill_n(opts.uinput_config.btn_map,  (int)XBOX_BTN_MAX,  ButtonEvent::invalid());
        }
      else if (strcmp(argv[i], "--ui-axismap") == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2apply(argv[i], boost::bind(&set_ui_axis_map, opts.uinput_config.axis_map, _1));
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }                  
        }
      else if (strcmp(argv[i], "--ui-buttonmap") == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2apply(argv[i], boost::bind(&set_ui_button_map, opts.uinput_config.btn_map, _1));
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }         
        }
      else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--id") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.controller_id = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-w") == 0 ||
               strcmp(argv[i], "--wid") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.wireless_id = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-l") == 0 ||
               strcmp(argv[i], "--led") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (strcmp(argv[i], "help") == 0)
                {
                  print_led_help();
                  exit(EXIT_SUCCESS);
                }
              else
                {
                  opts.led = atoi(argv[i]);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp("--dpad-only", argv[i]) == 0)
        {
          if (opts.uinput_config.dpad_as_button)
            throw std::runtime_error("Can't combine --dpad-as-button with --dpad-only");

          opts.uinput_config.dpad_only = true;
        }
      else if (strcmp("--dpad-as-button", argv[i]) == 0)
        {
          if (opts.uinput_config.dpad_only)
            throw std::runtime_error("Can't combine --dpad-as-button with --dpad-only");

          opts.uinput_config.dpad_as_button = true;
        }
      else if (strcmp("--deadzone", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.deadzone = to_number(32767, argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an INT argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp("--trigger-as-button", argv[i]) == 0)
        {
          if (opts.uinput_config.trigger_as_zaxis)
            {
              std::cout << "Error: Can't combine --trigger-as-button and --trigger-as-zaxis" << std::endl;
              exit(EXIT_FAILURE);
            }
          else
            {
              opts.uinput_config.trigger_as_button = true;
            }
        }
      else if (strcmp("--autofire", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2vector(argv[i], opts.autofire_map, &AutoFireMapping::from_string);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp("--calibration", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2vector(argv[i], opts.calibration_map, &CalibrationMapping::from_string);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp("--relative-axis", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              arg2vector(argv[i], opts.relative_axis_map, &RelativeAxisMapping::from_string);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp("--square-axis", argv[i]) == 0)
        {
          opts.square_axis = true;
        }
      else if (strcmp("--trigger-as-zaxis", argv[i]) == 0)
        {
          if (opts.uinput_config.trigger_as_button)
            {
              std::cout << "Error: Can't combine --trigger-as-button and --trigger-as-zaxis" << std::endl;
              exit(EXIT_FAILURE);
            }
          else
            {
              opts.uinput_config.trigger_as_zaxis = true;
            }
        }
      else if (strcmp("--help-led", argv[i]) == 0)
        {
          print_led_help();
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "--device-by-id") == 0)
        {
          ++i;
          if (i < argc)
            {
              unsigned int product_id;
              unsigned int vendor_id;
              if (sscanf(argv[i], "%x:%x", &vendor_id, &product_id) == 2)
                {
                  opts.vendor_id  = vendor_id;
                  opts.product_id = product_id;
                }
              else
                {
                  std::cout << "Error: " << argv[i-1] << " expected an argument in form PRODUCT:VENDOR (i.e. 046d:c626)" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }        
        }
      else if (strcmp(argv[i], "--device-by-path") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (sscanf(argv[i], "%3s:%3s", opts.busid, opts.devid) == 2)
                {
                }
              else
                {
                  std::cout << "Error: " << argv[i-1] << " expected an argument in form BUS:DEV (i.e. 006:003)" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "--list-controller") == 0 ||
               strcmp(argv[i], "-L") == 0)
        {
          usb_init();
          usb_find_busses();
          usb_find_devices();

          list_controller();
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "--help-devices") == 0)
        {
          std::cout << " idVendor | idProduct | Name" << std::endl;
          std::cout << "----------+-----------+---------------------------------" << std::endl;
          for(unsigned int i = 0; i < sizeof(xpad_devices)/sizeof(XPadDevice); ++i)
            {
              std::cout << boost::format("   0x%04x |    0x%04x | %s")
                % int(xpad_devices[i].idVendor)
                % int(xpad_devices[i].idProduct)
                % xpad_devices[i].name 
                        << std::endl;
            }
          exit(EXIT_SUCCESS);
        }
      else
        {
          std::cout << "Error: unknown command line option: " << argv[i] << std::endl;
          exit(EXIT_FAILURE);
        }
    }
}

void print_info(struct usb_device* dev,
                const XPadDevice& dev_type,
                const CommandLineOptions& opts)
{
  std::cout << "USB Device:        " << dev->bus->dirname << ":" << dev->filename << std::endl;
  std::cout << "Controller:        " << boost::format("\"%s\" (idVendor: 0x%04x, idProduct: 0x%04x)")
    % dev_type.name % uint16_t(dev->descriptor.idVendor) % uint16_t(dev->descriptor.idProduct) << std::endl;
  if (dev_type.type == GAMEPAD_XBOX360_WIRELESS)
    std::cout << "Wireless Port:     " << opts.wireless_id << std::endl;
  std::cout << "Controller Type:   " << dev_type.type << std::endl;
  std::cout << "Deadzone:          " << opts.deadzone << std::endl;
  std::cout << "Rumble Debug:      " << (opts.rumble ? "on" : "off") << std::endl;
  std::cout << "Rumble Speed:      " << "left: " << opts.rumble_l << " right: " << opts.rumble_r << std::endl;
  if (opts.led == -1)
    std::cout << "LED Status:        " << "auto" << std::endl;
  else
    std::cout << "LED Status:        " << opts.led << std::endl;

  std::cout << "Square Axis:       " << ((opts.square_axis) ? "yes" : "no") << std::endl;
  
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

namespace Math {
template<class T>
T clamp (const T& low, const T& v, const T& high)
{
  assert(low <= high);
  return std::max((low), std::min((v), (high)));
}
} // namespace Math

void squarify_axis_(int16_t& x_inout, int16_t& y_inout)
{
  if (x_inout != 0 || y_inout != 0)
    {
      // Convert values to float
      float x = (x_inout < 0) ? x_inout / 32768.0f : x_inout / 32767.0f;
      float y = (y_inout < 0) ? y_inout / 32768.0f : y_inout / 32767.0f;

      // Transform values to square range
      float l = sqrtf(x*x + y*y);
      float v = fabs((fabsf(x) > fabsf(y)) ? l/x : l/y);
      x *= v;
      y *= v;

      // Convert values to int16_t
      x_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((x < 0) ? x * 32768 : x * 32767), 32767));
      y_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((y < 0) ? y * 32768 : y * 32767), 32767));
    }
}

// Little hack to allow access to bitfield via reference
#define squarify_axis(x, y) \
{ \
  int16_t x_ = x;         \
  int16_t y_ = y;         \
  squarify_axis_(x_, y_); \
  x = x_;                 \
  y = y_;                 \
}

void apply_square_axis(XboxGenericMsg& msg)
{
  switch (msg.type)
    {
      case XBOX_MSG_XBOX:
        squarify_axis(msg.xbox.x1, msg.xbox.y1);
        squarify_axis(msg.xbox.x2, msg.xbox.y2);
        break;

      case XBOX_MSG_XBOX360:
        squarify_axis(msg.xbox360.x1, msg.xbox360.y1);
        squarify_axis(msg.xbox360.x2, msg.xbox360.y2);
        break;
        
      case XBOX_MSG_XBOX360_GUITAR:
        break;
    }
}

void apply_deadzone(XboxGenericMsg& msg, int deadzone)
{
  switch (msg.type)
    {
      case XBOX_MSG_XBOX:
        if (abs(msg.xbox.x1) < deadzone)
          msg.xbox.x1 = 0;
        if (abs(msg.xbox.y1) < deadzone)
          msg.xbox.y1 = 0;
        if (abs(msg.xbox.x2) < deadzone)
          msg.xbox.x2 = 0;
        if (abs(msg.xbox.y2) < deadzone)
          msg.xbox.y2 = 0;
        break;

      case XBOX_MSG_XBOX360:
        if (abs(msg.xbox360.x1) < deadzone)
          msg.xbox360.x1 = 0;
        if (abs(msg.xbox360.y1) < deadzone)
          msg.xbox360.y1 = 0;
        if (abs(msg.xbox360.x2) < deadzone)
          msg.xbox360.x2 = 0;
        if (abs(msg.xbox360.y2) < deadzone)
          msg.xbox360.y2 = 0;      
        break;

      case XBOX_MSG_XBOX360_GUITAR:
        // FIXME: any use for deadzone here?
        break;
    }
}

uint32_t get_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

void set_rumble(XboxGenericController* controller, int gain, uint8_t lhs, uint8_t rhs)
{
  lhs = std::min(lhs * gain / 255, 255);
  rhs = std::min(rhs * gain / 255, 255);
  
  //std::cout << (int)lhs << " " << (int)rhs << std::endl;

  controller->set_rumble(lhs, rhs);
}

void controller_loop(GamepadType type, uInput* uinput, XboxGenericController* controller, CommandLineOptions& opts)
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

  // FIXME: We could block in some cases instead of poll
  //if (autofire_modifier.get() ||
  //relative_axis_modifier.get() ||
  //    opts.uinput_config.force_feedback)
  timeout = 25; // FIXME: How long should we wait for a new event?

  memset(&oldmsg, 0, sizeof(oldmsg));
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

      apply_calibration_map(msg, opts.calibration_map);

      // Apply modifier
      apply_deadzone(msg, opts.deadzone);

      if (opts.square_axis)
        apply_square_axis(msg);

      if (autofire_modifier.get())
        autofire_modifier->update(msec_delta, msg);
      
      if (relative_axis_modifier.get())
        relative_axis_modifier->update(msec_delta, msg);

      if (!opts.button_map.empty())
        apply_button_map(msg, opts.button_map);

      if (!opts.axis_map.empty())
        apply_axis_map(msg,   opts.axis_map);

      if (memcmp(&msg, &oldmsg, sizeof(XboxGenericMsg)))
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
              else if (type == GAMEPAD_FIRESTORM)
                {
                  set_rumble(controller, opts.rumble_gain,
                             std::min(255, abs((msg.xbox360.y1>>8)*2)), 
                             std::min(255, abs((msg.xbox360.y2>>8)*2)));
                }
            }
        }

      uinput->update(msec_delta);
    }
}

void find_controller(struct usb_device*& dev,
                     XPadDevice&         dev_type,
                     const CommandLineOptions& opts)
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

int led_count = 0;

void on_sigint(int)
{
  if (global_exit_xboxdrv)
    {
      std::cout << "Ctrl-c pressed twice, exiting hard" << std::endl;
      exit(EXIT_SUCCESS);
    }
  else
    {
      std::cout << "Shutdown initiated, press Ctrl-c again if nothing is happening" << std::endl;
      global_exit_xboxdrv = true; 
      if (global_controller)
        global_controller->set_led(0);
    }
}

void run_main(CommandLineOptions& opts)
{
  if (!opts.quiet)
    {
      print_version();
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

      XboxGenericController* controller = 0;

      switch (dev_type.type)
        {
          case GAMEPAD_XBOX:
          case GAMEPAD_XBOX_MAT:
            controller = new XboxController(dev);
            break;

          case GAMEPAD_XBOX360_GUITAR:
            controller = new Xbox360Controller(dev, true);
            break;

          case GAMEPAD_XBOX360:
            controller = new Xbox360Controller(dev, false);
            break;

          case GAMEPAD_XBOX360_WIRELESS:
            controller = new Xbox360WirelessController(dev, opts.wireless_id);
            break;

          case GAMEPAD_FIRESTORM:
            controller = new FirestormDualController(dev);
            break;

          default:
            assert(!"Unknown gamepad type");
        }

      global_controller = controller;

      int jsdev_number = find_jsdev_number();
      int evdev_number = find_evdev_number();

      // FIXME: insert /dev/input/jsX detection magic here
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
          uInput* uinput = 0;
          if (!opts.no_uinput)
            {
              if (!opts.quiet)
                std::cout << "\nStarting with uinput... " << std::flush;
              uinput = new uInput(dev_type, opts.uinput_config);
              uinput->set_ff_callback(boost::bind(&set_rumble,  controller, opts.rumble_gain, _1, _2));
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
          controller_loop(dev_type.type, uinput, controller, opts);
          
          delete controller;
          delete uinput;
         
          if (!opts.quiet) 
            std::cout << "Shutdown complete" << std::endl;
        }
    }
}

int main(int argc, char** argv)
{
  try 
    {
      signal(SIGINT, on_sigint);

      CommandLineOptions opts;
      command_line_options = &opts;

      parse_command_line(argc, argv, opts);

      if (opts.daemon)
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
      else
        {
          run_main(opts);
        }
    }
  catch(std::exception& err)
    {
      std::cout << "Exception: " << err.what() << std::endl;
    }

  return 0;
}

/* EOF */
