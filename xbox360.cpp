#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>



int main(int argc, char** argv)
{
  struct usb_bus *busses;
  
  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next) 
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          std::cout << (boost::format("UsbDevice: idVendor: 0x%04x idProduct: 0x%04x")
                        % (int)dev->descriptor.idProduct 
                        % (int)dev->descriptor.idVendor)
                    << std::endl;

          if (dev->descriptor.idVendor  == 0x045e &&
              dev->descriptor.idProduct == 0x028e)
            {
              std::cout << " ^-- XBox360 Controller" << std::endl;
              
              struct usb_dev_handle* handle = usb_open(dev);
              if (!handle)
                {
                  std::cout << "Error opening XBox360 controller" << std::endl;
                }
              else
                {
                  sleep(1);

                  std::cout << "Sending LED off" << std::endl;
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
                  for(int i = 15; i < 256; ++i)
                    {
                      std::cout << i << std::endl;
                      char ledcmd[] = {1, 3, i}; 
                      usb_bulk_write(handle, 2, ledcmd, 3, 0);
                      sleep(3);
                    }

                  if (0)
                    for(int i = 0; i < 100; ++i)
                      {
                        char buffer[1024];
                        //                        std::cout << "Read: " << 
                        //usb_bulk_read(handle, int ep,  ???
                        //            buffer, 1024, 10) << std::endl;
                      }

                  sleep(10);
                  usb_close(handle);
                }
            }
          
          /* Loop through all of the configurations */
          for (int c = 0; c < dev->descriptor.bNumConfigurations; c++) 
            { /* Loop through all of the interfaces */
              for (int i = 0; i < dev->config[c].bNumInterfaces; i++) 
                { /* Loop through all of the alternate settings */
                  for (int a = 0; a < dev->config[c].interface[i].num_altsetting; a++) 
                    { /* Check if this interface is a printer */
                      
                    }
                }
            }
        }
    }

  return 0;
}

/* EOF */
