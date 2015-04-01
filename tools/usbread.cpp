
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
  else if ((argc == 4 || argc == 5 || argc == 6) && strcmp("cat", argv[1]) == 0)
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
