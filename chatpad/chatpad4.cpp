#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <stdexcept>

class Main
{
private:
  libusb_context*       m_ctx;
  libusb_device_handle* m_handle;

  uint8_t old_buttons;
  uint8_t old_dpad;

  bool m_running;

public:
  Main() :
    m_ctx(0),
    m_handle(0),
    old_buttons(0),
    old_dpad(0),
    m_running(false)
  {}

  ~Main()
  {
  }

  void init_libusb()
  {    
    if (libusb_init(&m_ctx) != 0)
    {
      throw std::runtime_error("Libusb went wrong");
    }

    std::cout << "Debug to max" << std::endl;
    libusb_set_debug(m_ctx, 3);
  }
  void set_configuration()
  {
    old_buttons = 0;
    old_dpad    = 0;

    if (m_running)
    {
      libusb_release_interface(m_handle, 0);
      libusb_release_interface(m_handle, 2);
    }

    int ret = libusb_set_configuration(m_handle, 1);

    switch(ret)
    {
      case 0:
        std::cout << "set_configuration(): success" << std::endl;
        break;

      default:
        std::cout << "set_configuration(): " << ret << std::endl;
        break;
    }

    if (m_running)
    {
      libusb_claim_interface(m_handle, 0);
      libusb_claim_interface(m_handle, 2);
    }
  }

  void init_device_handle()
  {
    m_handle = libusb_open_device_with_vid_pid(m_ctx, 0x045e, 0x028e);

    set_configuration();

    std::cout << "handle: " << m_handle << std::endl;
    if (!m_handle)
    {
      throw std::runtime_error("Couldn't find controller");
    }

    int err = libusb_claim_interface(m_handle, 2);
    std::cout << "Claim: " << err << std::endl;

    err = libusb_claim_interface(m_handle, 0);
    std::cout << "Claim: " << err << std::endl;

    m_running = true;
  }

  void usb_send(uint8_t  reqtype, 
                uint8_t  request, 
                uint16_t value,
                uint16_t index,
                uint16_t length,
                unsigned char* data,
                unsigned int timeout)
  {
    usleep(100000);

    std::cout << "send: " 
              << boost::format("%02x %02x %04x %04x %04x") % int(reqtype) % int(request) % int(value) % int(index) % int(length)
              << " { ";
    for(int i = 0; i < length; ++i)
    {
      std::cout << boost::format("%02x ") % int(data[i]);
    }
    std::cout << " }" << std::endl;

    int ret;
    ret = libusb_control_transfer(m_handle, 
                                  reqtype,
                                  request,
                                  value,
                                  index,
                                  data,
                                  length,
                                  0       // timeout
      );

    if (ret < 0)
    {
      std::cout << "ctrl_msg: " << ret << '"' << strerror(-ret) << '"' << std::endl;
    }
    std::cout << std::endl;
  }

  void usb_receive(uint8_t  reqtype, 
                   uint8_t  request, 
                   uint16_t value,
                   uint16_t index,
                   uint16_t length,
                   unsigned int timeout)
  {
    usleep(100000);

    std::cout << "recv: " 
              << boost::format("%02x %02x %04x %04x %04x") % int(reqtype) % int(request) % int(value) % int(index) % int(length)
              << " { ";

    unsigned char buf[1024] = { 0 };
    int ret;
    ret = libusb_control_transfer(m_handle, 
                                  reqtype,
                                  request,
                                  value,
                                  index,
                                  buf,
                                  length,
                                  0       // timeout
      );

    if (ret < 0)
    {
      std::cout << "ctrl_msg: " << ret << '"' << strerror(-ret) << '"' << std::endl;
    }
    else
    {
      for(int i = 0; i < ret; ++i)
      {
        std::cout << boost::format("%02x ") % int(buf[i]);
      }
      std::cout << " }" << std::endl;
    }

    std::cout << std::endl;
  }

  void run()
  {
    init_libusb();
    init_device_handle();

    if (true)
    {
      // my controller (seems constant):
      // 49 4b 00 00 17 b7 e8 06 39 02 ca 90 55 21 01 33 00 00 80 02 5e 04 8e 02 03 00 01 01 29
      // usb logs:
      // 49 4b 00 00 17 e7 d9 1d 50 02 20 85 55 21 01 33 00 00 80 02 5e 04 8e 02 03 00 01 01 c5
      //                ^^^^^^^^^^^    ^^^^^                                                 ^^
      usb_receive(0xC1, 0x81, 0x5B17, 0x0103, 0x001D, 0);

      {
        unsigned char data[] = { 
          0x09, 0x40, 0x00, 0x00, 0x1C, 0xDC, 0xA6, 0xB3,
          0xEF, 0x84, 0x3A, 0x2C, 0x8C, 0xB6, 0x57, 0xCC,
          0x09, 0xC7, 0x5E, 0x2F, 0x1B, 0x64, 0x58, 0x19,
          0xC6, 0x48, 0xED, 0x45, 0x37, 0xB7, 0xB5, 0x61,
          0xAA, 0x4C
        };
        usb_send(0x41, 0x82, 0x0003, 0x0103, 0x0022, data, 0);
      }

      // gives 00 00 or 01 00 or 02 00 
      usb_receive(0xC1, 0x86, 0x0000, 0x0103, 0x0002, 0);

      // gives 00 00 or 01 00 or 02 00 
      usb_receive(0xC1, 0x86, 0x0000, 0x0103, 0x0002, 0);

      usb_receive(0xC1, 0x83, 0x5C28, 0x0103, 0x002E, 0);

      {
        unsigned char data[] = { 
        };

        usb_send(0x41, 0x84, 0x0003, 0x0103, 0x0000, data, 0);
      }

      {
        unsigned char data[] = { 
          0x09, 0x41, 0x00, 0x00, 0x10, 0xBB, 0x4D, 0x42,
          0xA3, 0x40, 0x63, 0x24, 0x26, 0xD8, 0x10, 0x06,
          0x54, 0x47, 0xB2, 0x2F, 0xF7, 0x81
        };

        usb_send(0x41, 0x87, 0x0003, 0x0103, 0x0016, data, 0);
      }
    }

    usb_receive(0xC1, 0x86, 0x0000, 0x0103, 0x0002, 0); 
    usb_receive(0xC1, 0x86, 0x0000, 0x0103, 0x0002, 0);
    usb_receive(0xC1, 0x83, 0x5C10, 0x0103, 0x0016, 0);
    usb_receive(0xC0, 0x01, 0x0000, 0x0000, 0x0004, 0);

    usb_send(0x40, 0xa9, 0xa30c, 0x4423, 0x0000, NULL, 0);
    usb_send(0x40, 0xa9, 0x2344, 0x7f23, 0x0000, NULL, 0);
    usb_send(0x40, 0xa9, 0x5839, 0x6832, 0x0000, NULL, 0);

    usb_receive(0xC0, 0xA1, 0x0000, 0xE416, 0x0002, 0);
    {
      unsigned char data[] = { 0x01, 0x02 };
      usb_send(0x40, 0xA1, 0x0000, 0xE416, 0x0002, data, 0);
    }

    libusb_close(m_handle);
    libusb_exit(m_ctx);
  }
};

int main(int argc, char* argv[])
{
  try 
  {
    Main app;
    app.run();
  }
  catch(const std::exception& err)
  {
    std::cout << "Error: " << err.what() << std::endl;
  }

  return 0;
}

/* EOF */
