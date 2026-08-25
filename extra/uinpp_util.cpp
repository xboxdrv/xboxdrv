#include <iostream>
#include <linux/input.h>
#include <unistd.h>

#include "multi_device.hpp"

int main()
{
  uinpp::MultiDevice device;
  device.add_key(uinpp::DEVICEID_KEYBOARD, KEY_A);
  device.finish();

  while(1) {
    device.send(uinpp::DEVICEID_KEYBOARD, EV_KEY, KEY_A, 1);
    device.sync();
    device.send(uinpp::DEVICEID_KEYBOARD, EV_KEY, KEY_A, 0);
    device.sync();
    sleep(1);
  }

  return 0;
}

/* EOF */
