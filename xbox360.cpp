#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>

struct XBox360Msg
{
  // --------------------------
  unsigned int dummy1      :8;
  unsigned int dummy2      :8;

  // data[2] ------------------
  unsigned int dpad_up     :1;
  unsigned int dpad_down   :1;
  unsigned int dpad_left   :1;
  unsigned int dpad_right  :1;

  unsigned int start       :1;
  unsigned int select      :1;

  unsigned int stick_left  :1;
  unsigned int stick_right :1;

  // data[3] ------------------
  unsigned int lb          :1;
  unsigned int rb          :1;
  unsigned int mode        :1;
  unsigned int dummy3      :1;

  unsigned int a           :1;
  unsigned int b           :1;
  unsigned int y           :1;
  unsigned int x           :1;

  // data[4] ------------------
  unsigned int lt          :8;
  unsigned int rt          :8;

  // data[6] ------------------
  unsigned int x1          :16;
  unsigned int y1          :16;

  // data[10] -----------------
  unsigned int x2          :16;
  unsigned int y2          :16;

  // data[14]; ----------------
  unsigned int dummy4      :32;
  unsigned int dummy5      :16;
} __attribute__((__packed__));

int get_bit(uint8_t data, int bit)
{
  return (data & (1 << bit)) >> bit;
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
        }
    }
  return 0;
}

int main(int argc, char** argv)
{
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
              char ledcmd[] = {1, 3, 0}; 
              usb_bulk_write(handle, 2, ledcmd, 3, 0);
            }

          while(1)
            {
              uint8_t data[20];
              int ret = usb_bulk_read(handle, 1,
                                      (char*)data, 20, 0);
              if (ret == 20 && data[0] == 0x00 && data[1] == 0x14)
                {
                  std::cout << boost::format("  S1:(%6d, %6d)") 
                    % *((int16_t*)(data+6))
                    % *((int16_t*)(data+8));

                  std::cout << boost::format("  S2:(%6d, %6d)")
                    % *((int16_t*)(data+10))
                    % *((int16_t*)(data+12));
                          
                  std::cout << boost::format(" [u:%d|d:%d|l:%d|r:%d]")
                    % get_bit(data[2], 0)
                    % get_bit(data[2], 1)
                    % get_bit(data[2], 2)
                    % get_bit(data[2], 3);

                  std::cout << "  select:" << get_bit(data[2], 5);
                  std::cout << " mode:"    << get_bit(data[3], 2);
                  std::cout << " start:"   << get_bit(data[2], 4);

                  std::cout << "  sl:" << get_bit(data[2], 6);
                  std::cout << " sr:"  << get_bit(data[2], 7);

                  std::cout << "  A:" << get_bit(data[3], 4);
                  std::cout << " B:"  << get_bit(data[3], 5);
                  std::cout << " X:"  << get_bit(data[3], 7);
                  std::cout << " Y:"  << get_bit(data[3], 6);

                  std::cout << "  LB:" << get_bit(data[3], 0);
                  std::cout << " RB:" <<  get_bit(data[3], 1);

                  std::cout << boost::format("  LT:%3d RT:%3d")
                    % int(data[4])
                    % int(data[5]);

                  std::cout << "\r" << std::flush;                         
                }
              else
                {
                  std::cout << "Unknown data: bytes: " << ret 
                            << " Data: ";
                      
                  for(int j = 0; j < ret; ++j)
                    {
                      std::cout << boost::format("0x%02x ") % int(data[j]);
                    }
                  std::cout << "\r" << std::flush;
                }
            }

          usb_close(handle);
        }
    }
  return 0;
}

/* EOF */
