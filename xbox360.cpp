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
              // Last byte is LED status:
              // 0: off
              // 1: all blinking
              // 2: top-left blink
              // 3: top-right blink
              // 4: bottom-left blink
              // 5: bottom-right blink
              // 6: top-left on
              // 7: top-right on
              // 8: bottom-left on
              // 9: bottom-right on
              // 10: rotate
              // 11: blink
              // 12: blink slower
              // 13: rotate with two lights
              // 14: blink
              // 15: blink once
              char ledcmd[] = {1, 3, 6}; 
              usb_bulk_write(handle, 2, ledcmd, 3, 0);
            }

          uInput* uinput = new uInput();
          while(!sigint_recieved)
            {
              uint8_t data[20];
              int ret = usb_bulk_read(handle, 1,
                                      (char*)data, 20, 0);
              XBox360Msg& msg = (XBox360Msg&)data;
              if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
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

                  // std::cout << " Dummy: " << msg.dummy3 << " " << msg.dummy4 << " " << msg.dummy5 << std::endl;

                  if (0) std::cout << "\r" << std::flush;
                  else   std::cout << std::endl;

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
