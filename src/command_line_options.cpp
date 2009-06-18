/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#include <iostream>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "helper.hpp"
#include "command_line_options.hpp"

CommandLineOptions* command_line_options = 0;

CommandLineOptions::CommandLineOptions()
{
  mode     = RUN_DEFAULT;

  verbose  = false;
  silent   = false;
  quiet    = false;
  rumble   = false;
  led      = -1;
  rumble_l = -1;
  rumble_r = -1;
  rumble_gain = 255;
  controller_id = 0;
  wireless_id   = 0;
  instant_exit = false;
  no_uinput = false;
  gamepad_type = GAMEPAD_UNKNOWN;
  busid[0] = '\0';
  devid[0] = '\0';
  vendor_id  = -1;
  product_id = -1;
  deadzone = 0;
  deadzone_trigger = 0;
  square_axis  = false;
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
          throw std::runtime_error("Couldn't convert string \"" + str + "\" to ui-axis-mapping");
        }      
    }  
}

void
CommandLineOptions::parse_args(int argc, char** argv)
{  
  CommandLineOptions& opts = *this;

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
          opts.mode = RUN_DAEMON;
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
      else if (strcmp("--deadzone-trigger", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.deadzone_trigger = to_number(255, argv[i]);
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
      else if (strcmp(argv[i], "--list-supported-devices") == 0)
        {
          for(int i = 0; i < xpad_devices_count; ++i)
            {
              std::cout << boost::format("%s 0x%04x 0x%04x %s\n")
                % gamepadtype_to_string(xpad_devices[i].type)
                % int(xpad_devices[i].idVendor)
                % int(xpad_devices[i].idProduct)
                % xpad_devices[i].name;
            }

          exit(EXIT_FAILURE);
        }
      else if (strcmp(argv[i], "--list-controller") == 0 ||
               strcmp(argv[i], "-L") == 0)
        {
          opts.mode = RUN_LIST_CONTROLLER;
        }
      else if (strcmp(argv[i], "--help-devices") == 0)
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
          exit(EXIT_SUCCESS);
        }
      else
        {
          std::cout << "Error: unknown command line option: " << argv[i] << std::endl;
          exit(EXIT_FAILURE);
        }
    }
}

void
CommandLineOptions::print_command_line_help(int argc, char** argv) const
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
  std::cout << "  --list-supported-devices list supported devices (used by xboxdrv-daemon.py)" << std::endl;
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
  std::cout << "  --deadzone-trigger INT   Threshold under which trigger events are ignored (default: 0)" << std::endl;
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
  std::cout << "  --calibration MAP        Changes the calibration for the given axis (example: X2=-32768:0:32767)" << std::endl;
  std::cout << "  --force-feedback         Enable force feedback support" << std::endl;
  std::cout << "  --rumble-gain NUM        Set relative rumble strength (default: 255)" << std::endl;
  std::cout << std::endl;
  std::cout << "See README for more documentation and examples." << std::endl;
  std::cout << "Report bugs to Ingo Ruhnke <grumbel@gmx.de>" << std::endl;
}

void
CommandLineOptions::print_led_help() const
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

void
CommandLineOptions::print_version() const
{
  std::cout
    << "xboxdrv 0.4.7\n"
    << "Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>\n"
    << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
    << "This is free software: you are free to change and redistribute it.\n"
    << "There is NO WARRANTY, to the extent permitted by law."
    << std::endl;
}

/* EOF */
