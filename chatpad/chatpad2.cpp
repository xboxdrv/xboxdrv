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

  void reset()
  {
    std::cout << "reset()" << std::endl;
    libusb_reset_device(m_handle);
    libusb_close(m_handle);
    libusb_exit(m_ctx);
    m_handle = 0;
    m_ctx    = 0;
    old_buttons = 0;
    old_dpad    = 0;

    execl("./chatpad2", "./chatpad2", NULL);
  }

  void ctrl_msg(uint8_t value)
  {
    int ret = libusb_control_transfer(m_handle,
                                      LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
                                      LIBUSB_REQUEST_GET_STATUS,
                                      value, 0x02, NULL, 0, 0);
    std::cout << "ctrl_msg: " << ret << boost::format(" %02x") % int(value) << std::endl;
  }

  void main_loop()
  {
    if (0)
    {
      ctrl_msg(0x1f);
      sleep(1);
      
      ctrl_msg(0x1e);
      sleep(1);

      ctrl_msg(0x1b);
      sleep(1);

      ctrl_msg(0x1b);
      sleep(1);

      ctrl_msg(0x1f);
      sleep(1);

      ctrl_msg(0x18);
      sleep(1);

      ctrl_msg(0x10);
      sleep(1);

      ctrl_msg(0x03);
      sleep(1);
    }

    while(true)
    {
      sleep(1);
    }
  }

  void read_thread(const std::string& prefix, int endpoint, boost::function<void (uint8_t*)> callback)
  {
    boost::scoped_array<uint8_t> data;

    int data_len = 32;
    if (endpoint == 6)
    {
      data.reset(new uint8_t[32]);
    }
    else 
    {
      data.reset(new uint8_t[32]);
    }

    while(true)
    {
      //std::cout << "reading" << std::endl;
      int transfered = -1;

      //int ret = libusb_interrupt_transfer(m_handle, 6 | LIBUSB_ENDPOINT_IN, 
      //data, sizeof(data),
      //&transfered, 0);

      int ret = libusb_interrupt_transfer(m_handle, endpoint | LIBUSB_ENDPOINT_IN, 
                                          data.get(), data_len,
                                          &transfered, 0);
      switch(ret)
      {
        case 0: // success
          {
            if (transfered != 20)
            {
              std::cout << prefix << "read(" << endpoint << "): " << transfered << "/32: data: " << std::flush;
              for(int i = 0; i < transfered; ++i)
              {
                std::cout << boost::format("0x%02x ") % int(data[i]);
              }
              std::cout << std::endl;
            }

            if (callback)
            {
              callback(data.get());
            }
            
            //            if (endpoint == 6)
            //  std::cout << "Clear Halt: " << libusb_clear_halt(m_handle, endpoint) << std::endl;
          }
          break;

        case LIBUSB_ERROR_TIMEOUT:
          std::cout << "read_thread: timeout" << std::endl;
          break;

        case LIBUSB_ERROR_PIPE:
          std::cout << "read_thread: pipe" << std::endl;
          break;

        case LIBUSB_ERROR_OVERFLOW:
          std::cout << "read_thread: overflow" << std::endl;
          break;

        case LIBUSB_ERROR_NO_DEVICE:
          std::cout << "read_thread: no device" << std::endl;
          break;
        
        case LIBUSB_ERROR_OTHER: // happens on reset
          std::cout << "read_thread: other error" << std::endl;
          break;

        default:
          std::cout << "read_thread: unknown: " << ret << std::endl;
          break;
      }
    }
  }

  void process_input(uint8_t* data)
  {
    uint8_t buttons = data[3];
    uint8_t dpad    = data[2];

    // only keep data that changed
    buttons = buttons & (buttons ^ old_buttons);
    dpad    = dpad    & (dpad    ^ old_dpad);

    old_buttons = buttons;
    old_dpad    = dpad;

    switch(dpad)
    {
      case 0x04: // left
        ctrl_msg(0x08);
        break;

      case 0x08: // right
        ctrl_msg(0x09);
        break;

      case 0x01: // up        
        ctrl_msg(0x0a);
        break;

      case 0x02: // down
        ctrl_msg(0x0b);
        break;

      case 0x20: // back
        break;

      case 0x10: // start
        set_configuration();
        break;

      case 0x40: // left stick
        break;

      case 0x80: // right stick
        break;
    }

    switch(buttons)
    {
      case 0x10: // a
        ctrl_msg(0x1f);
        break;

      case 0x20: // b
        ctrl_msg(0x1e);
        break;

      case 0x40: // x
        ctrl_msg(0x1b);
        break;

      case 0x80: // y
        ctrl_msg(0x10);
        break;

      case 0x01: // lb
        ctrl_msg(0x05);
        break;

      case 0x02: // rb
        ctrl_msg(0x06);
        break;

      case 0x04: // guide
        reset();
        break;
    }
  }

  void run()
  {
    init_libusb();
    init_device_handle();

    {
      boost::thread thread(boost::bind(&Main::read_thread, this, "cpad: ", 6, boost::function<void (uint8_t*)>()));
      
      boost::function<void (uint8_t*)> cb = boost::bind(&Main::process_input, this, _1);
      boost::thread thread2(boost::bind(&Main::read_thread, this, "data: ", 1, cb));
      main_loop();
    }

    libusb_close(m_handle);
    libusb_exit(m_ctx);
  }
};

int main()
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
