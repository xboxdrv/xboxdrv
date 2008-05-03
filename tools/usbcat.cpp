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

#include <usb.h>
#include <boost/format.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& out, struct usb_device* dev)
{
  if (dev)
    {
      char iProduct_buf[1024] = "(null)";

      // Little dirty to open usb device here
      struct usb_dev_handle* handle = usb_open(dev);
      if (handle)
        {
          if (dev->descriptor.iProduct) 
            usb_get_string_simple(handle, dev->descriptor.iProduct, iProduct_buf, sizeof(iProduct_buf));
          usb_close(handle);
        }
      
      return out << boost::format("idVendor: 0x%04hx  idProduct: 0x%04hx  iProduct: %s") 
        % uint16_t(dev->descriptor.idVendor) % uint16_t(dev->descriptor.idProduct)
        % iProduct_buf;
    }
  else
    {
      return out << "(usb_device: null)";
    }  
}

struct usb_device*
find_usb_device(uint16_t idVendor, uint16_t idProduct)
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          if (dev->descriptor.idVendor  == idVendor &&
              dev->descriptor.idProduct == idProduct)
            {
              return dev;
            }
        }
    }
  return 0;
}

void
cat_usb_device(struct usb_device* dev, int interface, int endpoint)
{
  struct usb_dev_handle* handle = usb_open(dev);
  if (!handle)
    {
      std::cout << "Error opening usb device" << std::endl;
    }
  else
    {
      if (usb_claim_interface(handle, interface) != 0) // FIXME: bInterfaceNumber shouldn't be hardcoded
        {
          std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
          if (usb_detach_kernel_driver_np(handle, interface) < 0)
            {
              std::cout << "Failure to kick kernel driver: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);              
            }

          if (usb_claim_interface(handle, interface) != 0) // FIXME: bInterfaceNumber shouldn't be hardcoded
            {
              std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);
            }
        }


      bool quit = false;

      while(!quit)
        {
          uint8_t data[1024];
          int ret = usb_interrupt_read(handle, endpoint, (char*)data, sizeof(data), 0);
          if (ret < 0)
            {
              std::cout << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
              std::cout << "Shutting down" << std::endl;
              quit = true;
            }
          else
            {
              std::cout << "len: " << ret 
                        << " data: ";
                      
              for(int j = 0; j < ret; ++j)
                {
                  std::cout << boost::format("0x%02x ") % int(data[j]);
                }
              //std::cout << "\r" << std::flush;
              std::cout << std::endl;
            }

          if (0)
            {
              int len = rand() % 10;
              char rumblecmd[len];
              for (int i = 0; i < len; ++i)
                {
                  rumblecmd[i] = rand() % 255;
                }

              std::cout << "Writing random data" << std::endl;
              if (usb_interrupt_write(handle, 0, rumblecmd, len, 0) < 0)
                {
                  std::cout << "Write Error: " << usb_strerror() << std::endl;
                }
            }
        }
      usb_release_interface(handle, interface);
      usb_close(handle);
    }
}

void
list_usb_devices()
{
  struct usb_bus* busses = usb_get_busses();

  int bus_idx = 0;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          std::cout << boost::format("Bus %s Device %s ") % bus->dirname % dev->filename
                    << " " << dev << std::endl;
        }
      bus_idx += 1;
    }
}

int main(int argc, char** argv)
{
  srand(time(NULL));

  if (argc == 2 && strcmp("list", argv[1]) == 0)
    {
      usb_init();
      usb_find_busses();
      usb_find_devices();

      list_usb_devices();
    }
  else if ((argc == 4 || argc == 5) && strcmp("cat", argv[1]) == 0)
    {
      uint16_t idVendor;
      uint16_t idProduct;
      int interface = 0;
      int endpoint  = 1;

      if (sscanf(argv[2], "0x%hx", &idVendor) == 1 &&
          sscanf(argv[3], "0x%hx", &idProduct) == 1)
        {
          if (argc == 5)
            interface = atoi(argv[4]);
          
          if (argc == 6)
            endpoint  = atoi(argv[5]);

          usb_init();
          usb_find_busses();
          usb_find_devices();

          struct usb_device* dev = find_usb_device(idVendor, idProduct);
          if (!dev)
            {
              std::cout << "Error: Device (" << boost::format("idVendor: 0x%04hx, idProduct: 0x%04hx") 
                % idVendor % idProduct << ") not found" << std::endl;
            }
          else
            {
              std::cout << "Reading data from: " << dev << " Interface: " << interface << " Endpoint: " << endpoint << std::endl; 
              cat_usb_device(dev, interface, endpoint);
            }
        }
      else
        {
          std::cout << "Error: Expected IDVENDOR IDPRODUCT" << std::endl;
        }
    }
  else
    {
      std::cout << "Usage: " << argv[0] << " list\n"
                << "       " << argv[0] << " cat IDVENDOR IDPRODUCT [INTERFACE] [ENDPOINT]"
                << std::endl;
    }
}

/* EOF */
