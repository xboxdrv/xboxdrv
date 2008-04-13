/* 
**  XBox360 USB Gamepad Userspace Driver
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

#include <signal.h>
#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>

#include "uinput.hpp"
#include "xbox360.hpp"

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
  { GAMEPAD_XBOX360,          0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },

  // Do these work?
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" },

  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR dancepad" },
  { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" },
};

const int xpad_devices_count = sizeof(xpad_devices)/sizeof(XPadDevice);

std::ostream& operator<<(std::ostream& out, const GamepadType& type) 
{
  switch (type)
    {
      case GAMEPAD_XBOX360:
        return out << "XBox360";

      case GAMEPAD_XBOX360_WIRELESS:
        return out << "XBox360 (wireless)";

      case GAMEPAD_XBOX:
        return out << "XBox Classic";

      case GAMEPAD_XBOX_MAT:
        return out << "XBox Dancepad";
        
      default:
        return out << "unknown" << std::endl;
    }
}

std::ostream& operator<<(std::ostream& out, const XBox360Msg& msg) 
{
  out << boost::format("  S1:(%6d, %6d)") 
    % int(msg.x1) % int(msg.y1);

  out << boost::format("  S2:(%6d, %6d)")
    % int(msg.x2) % int(msg.y2);
                          
  out << boost::format(" [u:%d|d:%d|l:%d|r:%d]")
    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right);

  out << "  back:" << msg.select;
  out << " mode:"  << msg.mode;
  out << " start:" << msg.start;

  out << "  sl:" << msg.thumb_l;
  out << " sr:"  << msg.thumb_r;

  out << "  A:" << msg.a;
  out << " B:"  << msg.b;
  out << " X:"  << msg.x;
  out << " Y:"  << msg.y;

  out << "  LB:" << msg.lb;
  out << " RB:" <<  msg.rb;

  out << boost::format("  LT:%3d RT:%3d")
    % int(msg.lt) % int(msg.rt);

  if (0)
    out << " Dummy: " << msg.dummy3 << " " << msg.dummy4 << " " << msg.dummy5;

  return out;
}

std::ostream& operator<<(std::ostream& out, const XBoxMsg& msg) 
{
  out << boost::format(" S1:(%6d, %6d) S2:(%6d, %6d) "
                       " [u:%d|d:%d|l:%d|r:%d] "
                       " start:%d back:%d "
                       " sl:%d sr:%d "
                       " A:%3d B:%3d X:%3d Y:%3d "
                       " black:%3d white:%3d "
                       " LT:%3d RT:%3d ")
    % int(msg.x1) % int(msg.y1)
    % int(msg.x2) % int(msg.y2)

    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right)

    % int(msg.start)
    % int(msg.back)

    % int(msg.thumb_l)
    % int(msg.thumb_r)

    % int(msg.a)
    % int(msg.b)
    % int(msg.x)
    % int(msg.y)

    % int(msg.black)
    % int(msg.white)

    % int(msg.lt) 
    % int(msg.rt);

  return out;
}

void list_controller()
{
  struct usb_bus* busses = usb_get_busses();

  int id = 0;
  std::cout << " id | idVendor | idProduct | Name" << std::endl;
  std::cout << "----+----------+-----------+---------------------------------" << std::endl;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          for(int i = 0; i < xpad_devices_count; ++i)
            {
              if (dev->descriptor.idVendor  == xpad_devices[i].idVendor &&
                  dev->descriptor.idProduct == xpad_devices[i].idProduct)
                {
                  std::cout << boost::format(" %2d |   0x%04x |    0x%04x | %s")
                    % id
                    % int(xpad_devices[i].idVendor)
                    % int(xpad_devices[i].idProduct)
                    % xpad_devices[i].name 
                            << std::endl;
                  id += 1;
                  break;
                }
            }
        }
    }

  if (id == 0)
    std::cout << "\nNo controller detected" << std::endl; 
}

bool find_xbox360_controller(int id, struct usb_device** xbox_device, XPadDevice** type)
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
                      *type        = &xpad_devices[i];
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

int main(int argc, char** argv)
{
  bool verbose = false;
  bool rumble  = false;
  char led     = 0;
  int  rumble_l = 0;
  int  rumble_r = 0;
  int  controller_id = 0;
  bool instant_exit = false;
  uInputCfg uinput_config;

  for(int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-h") == 0 ||
          strcmp(argv[i], "--help") == 0)
        {
          std::cout << "Usage: " << argv[0] << " [OPTION]..." << std::endl;
          std::cout << "XBox360 USB Gamepad Userspace Driver" << std::endl;
          std::cout << std::endl;
          std::cout << "Options: " << std::endl;
          std::cout << "  -h, --help               display this help and exit" << std::endl;
          std::cout << "  -v, --verbose            display controller events" << std::endl;
          std::cout << "  -l, --led NUM            set LED status, see --list-led-values (default: 0)" << std::endl;
          std::cout << "  -r, --rumble L,R         set the speed for both rumble motors [0-255] (default: 0,0)" << std::endl;
          std::cout << "  -i, --id N               controller number (default: 0)" << std::endl;
          std::cout << "  -q, --quit               only set led and rumble status then quit" << std::endl;
          std::cout << "  --trigger-as-button      LT and RT send button instead of axis events" << std::endl;
          std::cout << "  --test-rumble            map rumbling to LT and RT (for testing only)" << std::endl;
          std::cout << "  --list-devices           list supported devices" << std::endl;
          std::cout << "  --list-controller        list available controllers" << std::endl;
          std::cout << "  --list-led-values        list possible values for the led" << std::endl;

          std::cout << std::endl;
          std::cout << "Report bugs to Ingo Ruhnke <grumbel@gmx.de>" << std::endl;
          return EXIT_SUCCESS;
        }
      else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0)
        {
          verbose = true;
        }
      else if (strcmp(argv[i], "--test-rumble") == 0)
        {
          rumble = true;
        }
      else if (strcmp(argv[i], "-r") == 0 ||
               strcmp(argv[i], "--rumble") == 0)
        {
          ++i;
          if (i < argc)
            {
              sscanf(argv[i], "%d,%d", &rumble_l, &rumble_r);
              rumble_l = std::max(0, std::min(255, rumble_l));
              rumble_r = std::max(0, std::min(255, rumble_r));
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected a argument" << std::endl;
              return EXIT_FAILURE;
            }          
        }
      else if (strcmp(argv[i], "-q") == 0 ||
               strcmp(argv[i], "--quit") == 0)
        {
          instant_exit = true;
        }
      else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--id") == 0)
        {
          ++i;
          if (i < argc)
            {
              controller_id = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected a argument" << std::endl;
              return EXIT_FAILURE;
            }
        }
      else if (strcmp(argv[i], "-l") == 0 ||
               strcmp(argv[i], "--led") == 0)
        {
          ++i;
          if (i < argc)
            {
              led = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected a argument" << std::endl;
              return EXIT_FAILURE;
            }
        }
      else if (strcmp("--trigger-as-button", argv[i]) == 0)
        {
          uinput_config.trigger_as_button = true;
        }
      else if (strcmp("--list-led-values", argv[i]) == 0)
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
          return EXIT_SUCCESS;
        }
      else if (strcmp(argv[i], "--list-controller") == 0)
        {
          usb_init();
          usb_find_busses();
          usb_find_devices();

          list_controller();
          return EXIT_SUCCESS;
        }
      else if (strcmp(argv[i], "--list-devices") == 0)
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
          return EXIT_SUCCESS;
        }
      else
        {
          std::cout << "Error: unknown command line option: " << argv[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  struct usb_device* dev      = 0;
  XPadDevice*        dev_type = 0;
  if (!find_xbox360_controller(controller_id, &dev, &dev_type))
    {
      std::cout << "No XBox360 Controller found" << std::endl;
    }
  else 
    {
      // Could/should fork here to hande multiple controllers at once
      
      std::cout << "Controller:        " << boost::format("\"%s\" (idVendor: 0x%04x, idProduct: 0x%04x)")
        % dev_type->name % dev_type->idVendor % dev_type->idProduct << std::endl;
      std::cout << "Controller Type:   " << dev_type->type << std::endl;
      std::cout << "Rumble Debug:      " << (rumble ? "on" : "off") << std::endl;
      std::cout << "Rumble Speed:      " << "left: " << rumble_l << " right: " << rumble_r << std::endl;
      std::cout << "LED Status:        " << int(led) << std::endl;

      struct usb_dev_handle* handle = usb_open(dev);
      if (!handle)
        {
          std::cout << "Error opening XBox360 controller" << std::endl;
        }
      else
        {
          // Handle LED on XBox360 Controller
          if (dev_type->type == GAMEPAD_XBOX360)
            {
              char ledcmd[] = {1, 3, led}; 
              usb_bulk_write(handle, 2, ledcmd, 3, 0);
            }

          // Switch of Rumble
          if (dev_type->type == GAMEPAD_XBOX360 ||
              dev_type->type == GAMEPAD_XBOX360_WIRELESS)
            {
              char l = rumble_r; // light weight
              char b = rumble_l; // big weight
              char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
              usb_bulk_write(handle, 2, rumblecmd, 8, 0);
            }
          else if (dev_type->type == GAMEPAD_XBOX)
            {
              char l = rumble_l;
              char b = rumble_r;
              char rumblecmd[] = { 0x00, 0x06, 0x00, l, 0x00, b };
              usb_bulk_write(handle, 2, rumblecmd, 6, 0);              
            }

          if (instant_exit)
            {

            }
          else 
            {          
              uInput* uinput = new uInput(dev_type->type, uinput_config);
              std::cout << "\nYour XBox360 controller should now be available as /dev/input/jsX" << std::endl;
              std::cout << "Press Ctrl-c to quit" << std::endl;

              bool quit = false;
              uint8_t old_data[20];
              memset(old_data, 0, 20);
              while(!quit)
                {
                  uint8_t data[20];
                  int ret = usb_bulk_read(handle, 1,
                                          (char*)data, 20, 0);
                  if (ret < 0)
                    { // Error
                      std::cout << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
                      std::cout << "Shutting down" << std::endl;
                      quit = true;
                    }
                  else if (ret == 0)
                    {
                      // happen with the XBox360 every now and then, just
                      // ignore, seems harmless
                    }
                  else if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                    {
                      if (memcmp(data, old_data, 20) == 0)
                        {
                          // Ignore the data, since nothing has changed
                        }                
                      else
                        {
                          memcpy(old_data, data, 20);

                          if (dev_type->type == GAMEPAD_XBOX360 ||
                              dev_type->type == GAMEPAD_XBOX360_WIRELESS)  
                            {
                              XBox360Msg& msg = (XBox360Msg&)data;

                              if (verbose)
                                std::cout << msg << std::endl;

                              uinput->send(msg);
                    
                              if (rumble)
                                {
                                  char l = msg.rt;
                                  char b = msg.lt;
                                  char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
                                  usb_bulk_write(handle, 2, rumblecmd, 8, 0);
                                }

                            }
                          else if (dev_type->type == GAMEPAD_XBOX)
                            { 
                              XBoxMsg& msg = (XBoxMsg&)data;                

                              if (verbose)
                                std::cout << msg << std::endl;

                              uinput->send(msg);

                              if (rumble)
                                {
                                  char l = msg.lt;
                                  char b = msg.rt;
                                  char rumblecmd[] = { 0x00, 0x06, 0x00, l, 0x00, b };
                                  usb_bulk_write(handle, 2, rumblecmd, 6, 0);
                                }
                            }    
                        }                  
                    }
                  else
                    {
                      /* Happens with XBox360 Controller sometimes
                         Unknown data: bytes: 3 Data: 0x01 0x03 0x0e 
                         Unknown data: bytes: 3 Data: 0x02 0x03 0x00 
                         Unknown data: bytes: 3 Data: 0x03 0x03 0x03 
                         Unknown data: bytes: 3 Data: 0x08 0x03 0x00 
                         -- different session:
                         Unknown data: bytes: 3 Data: 0x01 0x03 0x0e 
                         Unknown data: bytes: 3 Data: 0x02 0x03 0x00 
                         Unknown data: bytes: 3 Data: 0x03 0x03 0x03 
                         Unknown data: bytes: 3 Data: 0x08 0x03 0x00 
                         Unknown data: bytes: 3 Data: 0x01 0x03 0x06 

                      */

                      std::cout << "Unknown data: bytes: " << ret 
                                << " Data: ";
                      
                      for(int j = 0; j < ret; ++j)
                        {
                          std::cout << boost::format("0x%02x ") % int(data[j]);
                        }
                      //std::cout << "\r" << std::flush;
                      std::cout << std::endl;
                    }
                }
            }

          // Almost never reached since the user will Ctrl-c and we
          // can't use sigint since we block in usb_bulk_read()
          usb_close(handle);
        }
    }

  std::cout << "Done" << std::endl;
  return 0;
}

/* EOF */
