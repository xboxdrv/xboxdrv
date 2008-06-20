#include <boost/format.hpp>
#include <usb.h>
#include <sstream>
#include <pthread.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

class USBDevice;
class EndpointListenerThread;

void print_raw_data(std::ostream& out, uint8_t* data, int len);

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

class USBDevice
{
private:
  static USBDevice* current_;
public:
  static USBDevice* current() { return current_; }

private:
  struct usb_device*     dev;
  struct usb_dev_handle* handle;

  std::vector<EndpointListenerThread*> threads;
public: 
  USBDevice(struct usb_device* dev_)
    : dev(dev_)
  {
    USBDevice:: current_ = this;

    handle = usb_open(dev);
    if (!handle)
      {
        throw std::runtime_error("Error opening usb device");
      }
  }

  ~USBDevice()
  {
    usb_close(handle);
  }

  void claim_interface(int iface)
  {
    if (usb_claim_interface(handle, iface) != 0)
      {
        std::ostringstream str;
        str << "Couldn't claim interface " << iface;
        throw std::runtime_error(str.str());
      }
  }

  int read(int endpoint, uint8_t* data, int len)
  {
    return usb_interrupt_read(handle, endpoint, (char*)data, len, 0);
  }

  int write(int endpoint, uint8_t* data, int len)
  {
    return usb_interrupt_write(handle, endpoint, (char*)data, len, 0);
  }
  
  void print_info()
  {
    for(int i = 0; i < dev->descriptor.bNumConfigurations; ++i)
      {
        std::cout << "Configuration: " << i << std::endl;
        for(int j = 0; j < dev->config[i].bNumInterfaces; ++j)
          {
            std::cout << "  Interface " << j << ":" << std::endl;
            for(int k = 0; k < dev->config[i].interface[j].num_altsetting; ++k)
              {
                for(int l = 0; l < dev->config[i].interface[j].altsetting[k].bNumEndpoints; ++l)
                  {
                    std::cout << "    Endpoint: " 
                              << int(dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK)
                              << ((dev->config[i].interface[j].altsetting[k].endpoint[l].bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? " (IN)" : " (OUT)")
                              << std::endl;
                  }
              }
          }
      }
  }

  void launch_listener_thread(int endpoint);
};

USBDevice* USBDevice::current_ = 0;

class EndpointListenerThread
{
private:
  pthread_t thread;
  int  endpoint;
  bool running;
public:
  EndpointListenerThread(int endpoint_)
    : endpoint(endpoint_),
      running(false)
  {
  }

  ~EndpointListenerThread()
  {
    
  }

  void start()
  {
    running = true;
    pthread_create(&thread, NULL, &EndpointListenerThread::callback, this);
  }

  static void* callback(void* userdata)
  {
    EndpointListenerThread* thread = static_cast<EndpointListenerThread*>(userdata);
    thread->run();
    pthread_exit(NULL);
  }

  void run()
  {
    bool this_quit = false;
    std::cout << "Reading from endpoint " << endpoint << std::endl;;
    while(!this_quit)
      {
        uint8_t data[8192];
        int ret = USBDevice::current()->read(endpoint, data, sizeof(data));
        if (ret < 0)
          {
            std::cerr << "USBError: " << ret << "\n" << usb_strerror() << std::endl;
            std::cerr << "Shutting down" << std::endl;
            this_quit = true;
          }
        else
          {
            std::cout << ">>> Ep" << endpoint << ": ";
            print_raw_data(std::cout, data, ret);
            std::cout << std::endl;
          }
      }
  }
};

void 
USBDevice::launch_listener_thread(int endpoint)
{
  try {
    EndpointListenerThread* thread = new EndpointListenerThread(endpoint);
    threads.push_back(thread);
    thread->start();
  } catch(std::exception& err) {
    std::cout << "Error: " << err.what() << std::endl;
  }
}

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

bool readline(const std::string& prompt,
              std::string& line)
{
  std::cout << prompt << std::flush;
  bool ret = !std::getline(std::cin, line).eof();
  //line = strip(line);
  return ret;
}

std::vector<std::string> 
tokenize(const std::string& str, const std::string& delimiters = " ")
{
  std::vector<std::string> tokens;  

  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }

  return tokens;
}

void print_raw_data(std::ostream& out, uint8_t* data, int len)
{
  std::cout << "[" << len 
            << "] { ";
      
  for(int i = 0; i < len; ++i)
    {
      std::cout << boost::format("0x%02x") % int(data[i]);
      if (i != len-1)
        std::cout << ", ";
    }

  std::cout << " }";
}

void console_listen_cmd(const std::vector<std::string>& args)
{
  if (args.size() < 2)
    {
      std::cout << "Usage: listen [ENDPOINT]..." << std::endl;
    }
  else
    {
      for(size_t i = 1; i < args.size(); ++i)
        {
          int endpoint = atoi(args[i].c_str());
          USBDevice::current()->launch_listener_thread(endpoint);
        }
    }
}

void console_claim_cmd(const std::vector<std::string>& args)
{
  if (args.size() < 2)
    {
      std::cout << "Usage: claim [INTERFACE]..." << std::endl;
    }
  else
    {
      for(size_t i = 1; i < args.size(); ++i)
        {
          int interface = atoi(args[i].c_str());
          try {
            USBDevice::current()->claim_interface(interface);
          } catch (std::exception& err) {
            std::cout << "Error: " << err.what() << std::endl;
            goto end;
          }
          std::cout << "Interface " << interface << " successfully claimed" << std::endl;
        end:;
        }
    }
}

void console_info_cmd(const std::vector<std::string>& args)
{
  USBDevice::current()->print_info();
}

void console_send_cmd(const std::vector<std::string>& args)
{
  if (args.size() < 3)
    {
      std::cout << "Usage: send [EP] [DATA]..." << std::endl;
    }
  else
    {
      int endpoint = atoi(args[1].c_str());
      std::vector<uint8_t> data;
      for(int i = 2; i < int(args.size()); ++i)
        {
          data.push_back(strtol(args[i].c_str(), NULL, 16));
        }

      std::cout << "Sending to endpoint " << endpoint << ": ";
      print_raw_data(std::cout, &*data.begin(), data.size());
      int ret = USBDevice::current()->write(endpoint, &*data.begin(), data.size());
      std::cout << " -> " << ret << std::endl;
    }
}

std::string strip_comment(const std::string& line)
{
  std::string::size_type p = line.find_first_of('#');
  if (p != std::string::npos)
    {
      return line.substr(0, p);
    }
  else
    {
      return line;
    }
}

bool quit = false;

void eval_console_cmd(const std::string& line_)
{
  std::string line = strip_comment(line_);

  std::vector<std::string> args = tokenize(line);
  if (args.empty())
    {
      
    }
  else
    {
      if (0)
        {
          for(int i = 0; i < int(args.size()); ++i)
            {
              std::cout << i << ":'" << args[i] << "' ";
            }
          std::cout << std::endl;
        }

      if (args[0] == "help")
        {
          std::cout << "help:\n   Print this help\n" << std::endl;
          std::cout << "quit:\n   Exit usbdebug\n" << std::endl;
          std::cout << "claim [INTERFACE]...\n   Claim the given interfaces\n" << std::endl;
          std::cout << "listen [ENDPOINT]...\n   On the given endpoints\n" << std::endl;
          std::cout << "info\n   Print some info on the current device\n" << std::endl;
          std::cout << "send [ENDPOINT] [DATA]...\n   Send data to an USB Endpoint\n" << std::endl;
        }
      else if (args[0] == "listen")
        {
          console_listen_cmd(args);
        }
      else if (args[0] == "info")
        {
          console_info_cmd(args);
        }
      else if (args[0] == "claim")
        {
          console_claim_cmd(args);
        }
      else if (args[0] == "send")
        {
          console_send_cmd(args);
        }
      else if (args[0] == "quit")
        {
          std::cout << "Exiting" << std::endl;
          quit = true;
        }
      else
        {
          std::cout << "Unknown command '" << args[0] << "', type 'help' to list all available commands" << std::endl;
        }
    }
}

void run_console()
{
  std::cout << "Type 'help' to list all available commands" << std::endl;
  
  std::string line;

  while (!quit && readline("\033[32musb\033[0m> ", line))
    {
      std::vector<std::string> cmds = tokenize(line, ";");
      for(std::vector<std::string>::iterator i = cmds.begin(); i != cmds.end(); ++i)
        eval_console_cmd(*i);
    }
  std::cout << std::endl; 
}

int main(int argc, char** argv)
{
  if (argc != 2)
    {
      std::cout << "Usage: " << argv[0] << " [idVendor:idProduct]" << std::endl;
    }
  else
    {
      usb_init();
      usb_find_busses();
      usb_find_devices();

      uint16_t idVendor;
      uint16_t idProduct;
      if (sscanf(argv[1], "%hx:%hx", &idVendor, &idProduct) == 2)
        {
          struct usb_device* dev = find_usb_device(idVendor, idProduct);
            
          if (dev)
            {
              std::cout << boost::format("Opening device with idVendor: 0x%h04x, idProduct: 0x%h04x") % idVendor % idProduct << std::endl;
              USBDevice* usbdev = new USBDevice(dev);
              run_console();
              delete usbdev;
            }
          else
            {
              std::cout << "Couldn't device with " << argv[1] << std::endl;
            }
        }
      else
        {
          std::cout << "Syntax error, must be idVendor:idProduct (ex: 045e:028e)" << std::endl;
        }
    }

  return 0;
}

/* EOF */
