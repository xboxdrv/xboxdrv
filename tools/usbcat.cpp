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
#include <math.h>
#include <string.h>
#include <stdio.h>

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
write_usb_device(struct usb_device* dev, int interface, int endpoint)
{
  struct usb_dev_handle* handle = usb_open(dev);
  if (!handle)
    {
      std::cout << "Error opening usb device" << std::endl;
    }
  else
    {
      if (usb_claim_interface(handle, interface) != 0)
        {
          std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
          if (usb_detach_kernel_driver_np(handle, interface) < 0)
            {
              std::cout << "Failure to kick kernel driver: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);              
            }

          if (usb_claim_interface(handle, interface) != 0)
            {
              std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);
            }
        }

      bool quit = false;

      while(!quit)
        {
          uint8_t data[32];
          
          if (1)
            {
              int ret = fread(data, sizeof(char), sizeof(data), stdin);

              std::cout << ret << std::endl;
              usb_interrupt_write(handle, endpoint, (char*)data, ret, 0);

            }
          else
            {
              int ret = sizeof(data);
              for(int i = 0; i < ret ; ++i)
                {
                  data[i] = int(127 * sin(float(i) / ret * M_PI*2)) + 127;
                  std::cout << ret << std::endl;
                }
              std::cout << ret << std::endl;
              usb_interrupt_write(handle, endpoint, (char*)data, ret, 0);

            }
        }
    }   
}

void
read_usb_device(struct usb_device* dev, int interface, int endpoint)
{
  struct usb_dev_handle* handle = usb_open(dev);
  if (!handle)
    {
      std::cout << "Error opening usb device" << std::endl;
    }
  else
    {
      if (usb_claim_interface(handle, interface) != 0)
        {
          std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
          if (usb_detach_kernel_driver_np(handle, interface) < 0)
            {
              std::cout << "Failure to kick kernel driver: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);              
            }

          if (usb_claim_interface(handle, interface) != 0)
            {
              std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);
            }
        }

      bool quit = false;

      while(!quit)
        {
          uint8_t data[8192];
          int ret = usb_interrupt_read(handle, endpoint, (char*)data, sizeof(data), 0);
          if (ret < 0)
            {
              std::cerr << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
              std::cerr << "Shutting down" << std::endl;
              quit = true;
            }

          fwrite(data, sizeof(char), ret, stdout);
        }
    }  
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
      std::cout << "Claming interface " << interface << std::endl;
      if (usb_claim_interface(handle, interface) != 0)
        {
          std::cout << "Claming interface " << interface << " success" << std::endl;

          std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
          if (usb_detach_kernel_driver_np(handle, interface) < 0)
            {
              std::cout << "Failure to kick kernel driver: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);              
            }

          if (usb_claim_interface(handle, interface) != 0)
            {
              std::cout << "Error claiming the interface: " << usb_strerror() << std::endl;
              exit(EXIT_FAILURE);
            }
        }


      bool quit = false;

      while(!quit)
        {
          uint8_t data[32];
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
          for(int i = 0; i < ret; ++i)
            data[i] = 255;
          int ret2 = usb_interrupt_write(handle, 4, (char*)data, ret, 0);
          printf("Ret2: %d\n", ret2);
            }
          
          if (0)
            {
              char arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 255 };
              for (int len = 3; len <= 8; ++len)
                {
                  // Sending random data:
                  for (int front = 0; front < 256; ++front)
                    {
                      for (size_t i = 0; i < sizeof(arr); ++i)
                        {
                          char ledcmd[] = { front, len, arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i], arr[i] }; 
                          printf("%d %d %d\n", len, front, arr[i]);
                          usb_interrupt_write(handle, 5, ledcmd, len, 0);

                          uint8_t data[32];
                          int ret = usb_interrupt_read(handle, endpoint, (char*)data, sizeof(data), 10);
                          if (ret == -110)
                            {
                              
                            }
                          else if (ret < 0)
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

                        }
                    }
                }
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
  else if ((argc == 4 || argc == 5 || argc == 6) && 
           (strcmp("cat", argv[1]) == 0 ||
            strcmp("read", argv[1]) == 0 ||
            strcmp("write", argv[1]) == 0))
    {
      uint16_t idVendor;
      uint16_t idProduct;
      int interface = 0;
      int endpoint  = 1;

      if (sscanf(argv[2], "0x%hx", &idVendor) == 1 &&
          sscanf(argv[3], "0x%hx", &idProduct) == 1)
        {
          if (argc >= 5)
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
              if (strcmp("cat", argv[1]) == 0)
                {
                  std::cout << "Reading data from: " << dev << " Interface: " << interface << " Endpoint: " << endpoint << std::endl; 
                  cat_usb_device(dev, interface, endpoint);
                }
              else if (strcmp("read", argv[1]) == 0)
                {
                  read_usb_device(dev, interface, endpoint);
                }
              else if (strcmp("write", argv[1]) == 0)
                {
                  write_usb_device(dev, interface, endpoint);
                }
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
