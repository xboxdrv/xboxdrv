#include <libusb-1.0/libusb.h>
#include <iostream>
#include <stdexcept>

int main()
{
  libusb_context*       ctx = 0;
  libusb_device_handle* handle = 0;

  if (libusb_init(&ctx) != 0)
  {
    throw std::runtime_error("Libusb went wrong");
  }

  handle = libusb_open_device_with_vid_pid(ctx, 0x045e, 0x028e);

  if (!handle)
  {
    std::cout << "Couldn't find device" << std::endl;
  }
  else
  {
    std::cout << "Resetting" << std::endl;
    libusb_reset_device(handle);
    libusb_close(handle);
  }
  libusb_exit(ctx);

  return 0;
}

