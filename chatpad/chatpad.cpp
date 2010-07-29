#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <usb.h>
#include <iostream>
#include <stdexcept>

struct usb_device* find_controller()
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next)
  {
    for (struct usb_device* dev = bus->devices; dev; dev = dev->next)
    {
      if (dev->descriptor.idVendor  == 0x045e &&
          dev->descriptor.idProduct == 0x028e)
      {
          return dev;
      }
    }
  }
  return 0;
}

void read_thread(struct usb_dev_handle* handle)
{
  uint8_t data[5];
  while(true)
  {
    std::cout << "reading" << std::endl;
    int len = usb_interrupt_read(handle, 6, reinterpret_cast<char*>(data), sizeof(data), 0);
    if (len < 0)
    {
      std::cout << "Error in read_thread" << std::endl;
      return;
    }
    else
    {
      std::cout << "read: " << len << "/32: data: " << std::flush;
      for(int i = 0; i < len; ++i)
      {
        std::cout << boost::format("0x%02x ") % int(data[i]);
      }
      std::cout << std::endl;
    }
  }
}

int main()
{
  try 
  {
    usb_init();
    usb_find_busses();
    usb_find_devices();

    struct usb_device* dev = find_controller();

    if (!dev)
    {
      throw std::runtime_error("Couldn't find controller");
    }
    else
    {
      std::cout << "Controller found, ready to go" << std::endl;

      struct usb_dev_handle* handle = usb_open(dev);
      if (!handle)
      {
        throw std::runtime_error("Failed to open controller");
      }

      int err = usb_claim_interface(handle, 2);
      std::cout << "Claim: " << err << std::endl;

      boost::thread thread(boost::bind(&read_thread, handle));

      while(true)
      {
        usb_control_msg(handle, 0x41, 0x0, 0x1f, 0x02, 0, NULL, 0);
        std::cout << "0x1f" << std::endl;
        sleep(1);
       
        usb_control_msg(handle, 0x41, 0x0, 0x1e, 0x02, 0, NULL, 0);
        std::cout << "0x1e" << std::endl;
        sleep(1);

        //usb_control_msg(handle, 0x41, 0x0, 0x15, 0x02, 0, NULL, 0);
        //std::cout << "led and long sleep" << std::endl;
        usb_control_msg(handle, 0x41, 0x0, 0x1b, 0x02, 0, NULL, 0);
        sleep(1);


        usb_control_msg(handle, 0x41, 0x0, 0x1b, 0x02, 0, NULL, 0);
        sleep(1);

        usb_control_msg(handle, 0x41, 0x0, 0x1b, 0x02, 0, NULL, 0);
        sleep(1);

        //std::usb_reset(usb_dev_handle *dev);
      }

      usb_close(handle);
    }
  }
  catch(const std::exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
  }
  
  return 0;
}

/* EOF */
