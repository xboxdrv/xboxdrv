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

XPadDevice xpad_device[] = {
  // Evil?
  { GAMEPAD_XBOX,             0x0000, 0x0000, "Generic X-Box pad" },
  { GAMEPAD_XBOX,             0xffff, 0xffff, "Chinese-made Xbox Controller" },
  
  { GAMEPAD_XBOX,             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0287, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft X-Box pad v2 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft Xbox Controller S" },
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
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR dancepad" },
  { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" },
};

/*
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

  out << "  select:" << msg.select;
  out << " mode:"    << msg.mode;
  out << " start:"   << msg.start;

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

struct usb_device* 
find_xbox360_controller()
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next) 
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          if (0)
            std::cout << (boost::format("UsbDevice: idVendor: 0x%04x idProduct: 0x%04x")
                          % (int)dev->descriptor.idProduct 
                          % (int)dev->descriptor.idVendor)
                      << std::endl;

          if (dev->descriptor.idVendor  == 0x045e &&
              dev->descriptor.idProduct == 0x028e)
            return dev;
          else if (dev->descriptor.idVendor  == 0x045e &&
                   dev->descriptor.idProduct == 0x0289)
            return dev;
          else if (dev->descriptor.idVendor  == 0x046d &&
                   dev->descriptor.idProduct == 0xca88)
            return dev;
        }
    }
  return 0;
}

bool sigint_recieved = false;

void sigint_handler(int)
{
  if (sigint_recieved)
    {
      std::cout << "SIGINT recieved twice, exiting hard" << std::endl;
      exit(EXIT_FAILURE);
    }
  else
    {
      std::cout << "SIGINT recieved, shutting down" << std::endl;
      sigint_recieved = true;
    }
}

int main(int argc, char** argv)
{
  bool verbose = false;
  bool rumble  = false;
  char led     = 0;

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
          std::cout << "  -l, --led NUM            set LED status, see README (default: 0)" << std::endl;
          std::cout << "  -r, --rumble             map rumbling to LT and RT (for testing only)" << std::endl;
          std::cout << std::endl;
          std::cout << "Report bugs to Ingo Ruhnke <grumbel@gmx.de>" << std::endl;
          return EXIT_SUCCESS;
        }
      else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0)
        {
          verbose = true;
        }
      else if (strcmp(argv[i], "-r") == 0 ||
               strcmp(argv[i], "--rumble") == 0)
        {
          rumble = true;
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
      else
        {
          std::cout << "Error: unknown command line option: " << argv[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  signal(SIGINT, sigint_handler);

  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  struct usb_device* dev = find_xbox360_controller();
  if (!dev)
    {
      std::cout << "No XBox360 Controller found" << std::endl;
    }
  else 
    {
      std::cout << "XBox360 Controller found" << std::endl;
      struct usb_dev_handle* handle = usb_open(dev);
      if (!handle)
        {
          std::cout << "Error opening XBox360 controller" << std::endl;
        }
      else
        {
          if (1)
            {
              /* The LED-off command for Xbox-360 controllers */
              char ledcmd[] = {1, 3, led}; 
              usb_bulk_write(handle, 2, ledcmd, 3, 0);
            }

          if (0) // stop rumble
            {
              char l = 0; // light weight
              char b = 0; // big weight
              char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
              usb_bulk_write(handle, 2, rumblecmd, 8, 0);
            }
          
          std::cout << "Rumble Debugging is " << (rumble ? "on" : "off") << std::endl;
          std::cout << "LED status is " << int(led) << std::endl;
          //uInput* uinput = new uInput();
          std::cout << "Your XBox360 controller should now be available as /dev/input/js0" << std::endl;
          std::cout << "Press Ctrl-C twice to quit" << std::endl;
          while(!sigint_recieved)
            {
              uint8_t data[20];
              int ret = usb_bulk_read(handle, 1,
                                      (char*)data, 20, 0);
              if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                {
                  //XBox360Msg& msg = (XBox360Msg&)data;
                  XBoxMsg& msg = (XBoxMsg&)data;
                  
                  if (verbose)
                    {
                      std::cout << msg;
                      if (0) std::cout << "\r" << std::flush;
                      else   std::cout << std::endl;
                    }

                  if (rumble)
                    {
                      if (0) // XBox360
                        {
                          char l = msg.lt;
                          char b = msg.rt;
                          char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
                          usb_bulk_write(handle, 2, rumblecmd, 8, 0);
                        }
                      else
                        { // XBox Classic
                          char l = msg.lt;
                          char b = msg.rt;
                          char rumblecmd[] = { 0x00, 0x06, 0x00, b, 0x00, l };
                          usb_bulk_write(handle, 2, rumblecmd, 8, 0);
                        }
                    }

                  //uinput->send(msg);
                }
              else
                {
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

          usb_close(handle);
        }
    }

  std::cout << "Done" << std::endl;
  return 0;
}

/* EOF */
