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

          if (1) // stop rumble
            {
              char l = 0; // light weight
              char b = 0; // big weight
              char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
              usb_bulk_write(handle, 2, rumblecmd, 8, 0);
            }
          
          std::cout << "Rumble Debugging is " << (rumble ? "on" : "off") << std::endl;
          std::cout << "LED status is " << int(led) << std::endl;
          uInput* uinput = new uInput();
          std::cout << "Your XBox360 controller should now be available as /dev/input/js0" << std::endl;
          std::cout << "Press Ctrl-C twice to quit" << std::endl;
          while(!sigint_recieved)
            {
              uint8_t data[20];
              int ret = usb_bulk_read(handle, 1,
                                      (char*)data, 20, 0);
              if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                {
                  XBox360Msg& msg = (XBox360Msg&)data;
                  
                  if (verbose)
                    {
                      std::cout << boost::format("  S1:(%6d, %6d)") 
                        % int(msg.x1) % int(msg.y1);

                      std::cout << boost::format("  S2:(%6d, %6d)")
                        % int(msg.x2) % int(msg.y2);
                          
                      std::cout << boost::format(" [u:%d|d:%d|l:%d|r:%d]")
                        % int(msg.dpad_up)
                        % int(msg.dpad_down)
                        % int(msg.dpad_left)
                        % int(msg.dpad_right);

                      std::cout << "  select:" << msg.select;
                      std::cout << " mode:"    << msg.mode;
                      std::cout << " start:"   << msg.start;

                      std::cout << "  sl:" << msg.thumb_l;
                      std::cout << " sr:"  << msg.thumb_r;

                      std::cout << "  A:" << msg.a;
                      std::cout << " B:"  << msg.b;
                      std::cout << " X:"  << msg.x;
                      std::cout << " Y:"  << msg.y;

                      std::cout << "  LB:" << msg.lb;
                      std::cout << " RB:" <<  msg.rb;

                      std::cout << boost::format("  LT:%3d RT:%3d")
                        % int(msg.lt) % int(msg.rt);

                      if (0)
                        std::cout << " Dummy: " << msg.dummy3 << " " << msg.dummy4 << " " << msg.dummy5 << std::endl;

                      if (0) std::cout << "\r" << std::flush;
                      else   std::cout << std::endl;
                    }

                  if (rumble)
                    {
                      char l = msg.lt;
                      char b = msg.rt;
                      char rumblecmd[] = { 0x00, 0x08, 0x00, b, l, 0x00, 0x00, 0x00 };
                      usb_bulk_write(handle, 2, rumblecmd, 8, 0);
                    }

                  uinput->send(msg);
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
