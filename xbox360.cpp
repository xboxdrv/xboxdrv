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
                      int i = 10;
                      std::cout << i << std::endl;
                      char ledcmd[] = {1, 3, i}; 
                      usb_bulk_write(handle, 2, ledcmd, 3, 0);
                    }

                  while(1)
                    {
                      uint8_t data[20];
                      int ret = usb_bulk_read(handle, 1,
                                              (char*)data, 20, 0);
                      if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                        {
                          std::cout << " u:" << int(data[2] & 0x01);
                          std::cout << " d:" << int((data[2] & 0x02) >> 1);
                          std::cout << " l:" << int((data[2] & 0x04) >> 2);
                          std::cout << " r:" << int((data[2] & 0x08) >> 3);

                          std::cout << "  select:" << int((data[2] & 0x20) >> 5);
                          std::cout << " mode:" << int((data[3] & 0x04) >> 2);
                          std::cout << " start:" << int((data[2] & 0x10) >> 4);

                          std::cout << "  sl:" << int((data[2] & 0x40) >> 6);
                          std::cout << " sr:" << int((data[2]) >> 7);

                          std::cout << "  A:" << int((data[3] & 0x10) >> 4);
                          std::cout << " B:" << int((data[3] & 0x20) >> 5);
                          std::cout << " X:" << int((data[3] & 0x80) >> 7);
                          std::cout << " Y:" << int((data[3] & 0x40) >> 6);

                          std::cout << "  LB:" << int((data[3] & 0x01));
                          std::cout << " RB:" << int((data[3] & 0x02) >> 1);

                          std::cout << boost::format("  X:%6d Y:%6d") 
                            % int((int16_t)(((int16_t)data[7] << 8) | (int16_t)data[6]))
                            % int((int16_t)(((int16_t)data[9] << 8) | data[8]));

                          std::cout << boost::format("  X2:%6d Y2:%6d")
                            % (int16_t)(((int16_t)data[11] << 8) | (int16_t)data[10])
                            % (int16_t)(((int16_t)data[13] << 8) | (int16_t)data[12]);
                          
                          std::cout << boost::format("  LT:%3d RT:%3d")
                            % int(data[4])
                            % int(data[5]);

                          std::cout << std::endl;                         
                        }
                      else
                        {
                          std::cout << "Unknown data: bytes: " << ret 
                                    << " Data: ";
                      
                          for(int j = 0; j < ret; ++j)
                            {
                              std::cout << boost::format("0x%02x ") % int(data[j]);
                            }
                          std::cout << std::endl;
                        }
                    }

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
