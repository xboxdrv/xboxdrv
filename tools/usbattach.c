/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char** argv)
{
  if (argc != 2)
    {
      printf("Usage: %s /dev/bus/usb/${BUS}/${DEV}\n", argv[0]);
      printf("Reconnect USB devices after they got disconnected\n");
      return EXIT_FAILURE;
    }
  else
    {
      int fd = open(argv[1], O_RDWR);

      if (fd < 0)
        {
          perror(argv[1]);
          return EXIT_FAILURE;
        }
      else
        {
          struct usbdevfs_ioctl command;

          command.ifno = 0; // interface number, does it matter?
          command.ioctl_code = USBDEVFS_CONNECT;
          command.data = NULL;

          int ret = ioctl(fd, USBDEVFS_IOCTL, &command);

          if (ret < 0)
            {
              printf("%s: Could not issue usb connect on %s: %s\n", argv[0], argv[1], strerror(errno));
              close(fd);
              return EXIT_FAILURE;
            }
          else
            {
              printf("%s: reconnect of %s successful\n", argv[0], argv[1]);
              close(fd);
              return EXIT_SUCCESS;
            }
        }
    }
}

/* EOF */
