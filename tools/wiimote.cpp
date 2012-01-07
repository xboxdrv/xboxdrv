#include <iostream>
#include <stdlib.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "argument required" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    int dev_id;

    dev_id = hci_get_route(NULL);
    if (dev_id < 0) 
    {
      perror("Device is not available");
      exit(1);
    }

    std::cout << "connecting to: " << argv[1] << std::endl;
    bdaddr_t bdaddr;
    str2ba(argv[1], &bdaddr);

    int dd;
    dd = hci_open_dev(dev_id);
    if (dd < 0)
    {
      perror("hci_open_dev() failed");
      exit(1);
    }

    if (hci_authenticate_link(dd, htobs(cr->conn_info->handle), 25000) < 0) 
    {
      perror("HCI authentication request failed");
      exit(1);
    }

    if (hci_close_dev(dd) < 0)
    {
      perror("hci_close_dev()");
      exit(1);
    }

    return 0;
  }
}

/* EOF */
