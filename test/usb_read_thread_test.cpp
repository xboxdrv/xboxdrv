#include <iostream>
#include <libusb.h>
#include <stdexcept>

#include "helper.hpp"
#include "usb_helper.hpp"
#include "usb_read_thread.hpp" 
#include "raise_exception.hpp"

int main(int argc, char** argv)
{
  g_logger.set_log_level(Logger::kDebug);

  int ret = libusb_init(NULL);
  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_init() failed: " << usb_strerror(ret));
  }

  libusb_device_handle* handle = libusb_open_device_with_vid_pid(NULL, 0x045e, 0x028e);
    
  ret = libusb_claim_interface(handle, 1);

  if (ret != LIBUSB_SUCCESS)
  {
    raise_exception(std::runtime_error, "libusb_claim_interface() failed: " << usb_strerror(ret));
  }

  log_info("handle ok");

  if (!handle)
  {
    raise_exception(std::runtime_error, "couldn't find device");
  }
  else
  {
    log_info("starting usb thread");

    unsigned char data[32];

    USBReadThread thread(handle, 1, 32);

    thread.start_thread();
    bool shutdown = false;
    int count = 0;
    while(!shutdown)
    {
      count += 1;
      int transferred;

      std::cout << count << std::endl;

      if (thread.read(data, sizeof(data), &transferred, 100) != LIBUSB_ERROR_TIMEOUT)
      {
        std::cout << count << " " << transferred << " - " << raw2str(data, transferred) << std::endl;
      }


      if (!shutdown && count > 30)
      {
        std::cout << "shutdown thread" << std::endl;
        shutdown = true;

        thread.stop_thread();
        std::cout << "thread stopped" << std::endl;
      }

      //usleep(1000 * 10);
    }
  }

  return 0;
}

/* EOF */
