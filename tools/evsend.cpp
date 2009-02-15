#include <iostream>
#include <linux/input.h>

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/** Simple program to send events to /dev/input/eventX devices, useful
    for toggling LEDs and such */
int main(int argc, char** argv)
{
  if (argc != 5)
    {
      std::cout << "Usage: " << argv[0] << " DEVICE TYPE CODE VALUE" << std::endl;
    }
  else
    {
      const char* filename = argv[1];
      struct input_event event;
      event.type  = atoi(argv[2]);
      event.code  = atoi(argv[3]);
      event.value = atoi(argv[4]);

      int fd = open(filename, O_RDWR);
      if (fd < 0)        
        {
          std::cout << argv[0] << ": couldn't access: " << filename << ": " << strerror(errno) << std::endl;
        }
      else
        {
          if (write(fd, &event, sizeof(event)) != sizeof(event))
            {
              std::cout << argv[0] << ": write error: " << filename << ": " << strerror(errno) << std::endl;
            }
          else
            {
              // std::cout << "Send: Event(type: " << event.type << ", code: " << event.code << ", value: " << event.value << ")" << std::endl;
            }
          close(fd);
        }
    }

  return 0;
}

/* EOF */

