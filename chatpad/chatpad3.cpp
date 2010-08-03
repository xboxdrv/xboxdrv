#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>

class Main
{
private:
  libusb_context*       m_ctx;
  libusb_device_handle* m_handle;

public:
  Main() :
    m_ctx(0),
    m_handle(0)
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

    std::cout << "handle: " << m_handle << std::endl;
    if (!m_handle)
    {
      throw std::runtime_error("Couldn't find controller");
    }

    int err = libusb_claim_interface(m_handle, 2);
    std::cout << "Claim: " << err << std::endl;

    err = libusb_claim_interface(m_handle, 0);
    std::cout << "Claim: " << err << std::endl;
  }

  void ctrl_msg(uint8_t value)
  {
    int ret = libusb_control_transfer(m_handle,
                                      LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
                                      LIBUSB_REQUEST_GET_STATUS,
                                      value, 0x02, NULL, 0, 0);
    std::cout << "ctrl_msg: " << ret << boost::format(" %02x") % int(value) << std::endl;
  }

  static void callback_wrap(libusb_transfer* transfer)
  {
    static_cast<Main*>(transfer->user_data)->callback(transfer);
  }

  void callback(libusb_transfer* transfer)
  {
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
      std::cout << "data transfer not completed: " << transfer->status << std::endl;
    }

    unsigned char* data = transfer->buffer;
    std::cout << "callback(" << transfer->actual_length << "): ";
    for(int i = 0; i < transfer->actual_length; ++i)
    {
      std::cout << boost::format("0x%02x ") % int(data[i]);
    }
    std::cout << std::endl;

    // done with the transfer, so clean it up
    // libusb_free_transfer(transfer);

    // request more data
    request_controller_data(LIBUSB_ENDPOINT_ADDRESS_MASK & transfer->endpoint);
  }

  static void control_callback_wrap(libusb_transfer* transfer)
  {
    static_cast<Main*>(transfer->user_data)->control_callback(transfer);
  }

  void control_callback(libusb_transfer* transfer)
  {
    
  }

  void request_controller_data(int endpoint)
  {
    libusb_transfer* transfer = libusb_alloc_transfer(0);
    if (!transfer)
    {
      std::cout << "Couldn't alloc transfer" << std::endl;
    }

    unsigned char* buffer = static_cast<unsigned char*>(malloc(8+32));

    libusb_fill_interrupt_transfer(transfer,
                                   m_handle,
                                   endpoint | LIBUSB_ENDPOINT_IN,
                                   buffer, // buffer
                                   8+32,    // length,
                                   &Main::callback_wrap,
                                   this, // userdata
                                   0     // timeout
      );

    transfer->flags = 
      LIBUSB_TRANSFER_FREE_BUFFER  |
      LIBUSB_TRANSFER_FREE_TRANSFER;

    int ret = libusb_submit_transfer(transfer);
    if (ret != 0)
    {
      std::cout << "Error with libusb_submit_transfer: " << ret << std::endl;
    }
  }

  void request_control(unsigned char value)
  {
    libusb_transfer* transfer = libusb_alloc_transfer(0);
    if (!transfer)
    {
      std::cout << "Couldn't alloc transfer" << std::endl;
    }

    unsigned char* buffer = static_cast<unsigned char*>(malloc(LIBUSB_CONTROL_SETUP_SIZE));
    libusb_fill_control_transfer(transfer, m_handle, buffer, 
                                 &Main::control_callback_wrap, 
                                 this,
                                 0);

    libusb_fill_control_setup(buffer, 
                              LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE,
                              LIBUSB_REQUEST_GET_STATUS,
                              value,
                              2,
                              0);

    transfer->flags = 
      LIBUSB_TRANSFER_FREE_BUFFER  |
      LIBUSB_TRANSFER_FREE_TRANSFER;

    int ret = libusb_submit_transfer(transfer);
    if (ret != 0)
    {
      std::cout << "Error with libusb_submit_transfer: " << ret << std::endl;
    }
  }

  void main_loop()
  {
    request_controller_data(1);
    request_controller_data(6);
    request_control(0x1f);
    request_control(0x1b);
    while(true)
    {
      std::cout << "Handle events: " << libusb_handle_events(m_ctx) << std::endl;
    }
  }

  void run()
  {
    init_libusb();
    init_device_handle();

    main_loop();

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
