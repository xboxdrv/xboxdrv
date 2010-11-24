#include <libusb-1.0/libusb.h>
#include <iostream>
#include <stdexcept>

int main()
{
  libusb_context*       ctx;
  libusb_device_handle* handle;

  if (libusb_init(&ctx) != 0)
  {
    throw std::runtime_error("Libusb went wrong");
  }
  else
  {
    libusb_set_debug(ctx, 3);

    handle = libusb_open_device_with_vid_pid(ctx, 0x045e, 0x028e);

    int ret = libusb_reset_device(handle);

    std::cout << "ret: " << ret << std::endl;

    libusb_close(handle);

    libusb_exit(ctx);

    return 0;
  }
}

/* EOF */
