#include <linux/input.h>

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

void process_device(int fd)
{
  int version;
  if (ioctl(fd, EVIOCGVERSION, &version) < 0)
    {
      perror("evtest: can't get version");
      exit(1);
    }

  if (1)
    {
      char phys[1024];
      if (ioctl(fd, EVIOCGNAME(sizeof(phys)), &phys) < 0)
        {
          perror("evtest: can't get name");
          exit(1);
        }
      std::cout << "Name:    " << phys << std::endl;
    }

  if (1)
    {
      char phys[1024];
      if (ioctl(fd, EVIOCGPHYS(sizeof(phys)), &phys) < 0)
        {
          perror("evtest: can't get phys");
          exit(1);
        }
      std::cout << "Phys:    " << phys << std::endl;
    }

  if (0)
    {
      char phys[1024];
      if (ioctl(fd, EVIOCGUNIQ(sizeof(phys)), &phys) < 0)
        {
          perror("evtest: can't get uniq");
          exit(1);
        }
      std::cout << "Uniq:    " << phys << std::endl;
    }



  std::cout << "Version: " << (version >> 16) << "." << ((version >> 8) & 0xff) << "." << (version & 0xff) << std::endl;

  /*  unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
      ioctl(fd, EVIOCGID, id);
      ioctl(fd, EVIOCGNAME(sizeof(name)), name);
      memset(bit, 0, sizeof(bit));
      ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);*/
}

/**
   evtestplus
    --list          List available devices
    --test DEVICE   Test device
    --info DEVICE   List properties of device
 */
int main(int argc, char** argv)
{
  if (argc != 2)
    {
      std::cout << "Usage: " << argv[0] << " DEVICE" << std::endl;
      exit(EXIT_FAILURE);
    }
  else
    {
      const char* filename = argv[1];

      int fd;
      if ((fd = open(filename, O_RDONLY)) < 0)
        {
          perror(filename);
        }
      else
        {
          process_device(fd);

          close(fd);
        }
    }

  return 0;
}

/* EOF */
