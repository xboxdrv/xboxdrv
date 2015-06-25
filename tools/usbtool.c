/* 
**  usbtool - simple tool to (dis)connect a usb devices from a kernel driver
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <assert.h>
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

void print_usage(int argc, char** argv)
{
      printf("Usage: %s connect    /dev/bus/usb/${BUS}/${DEV}\n", argv[0]);
      printf("       %s disconnect /dev/bus/usb/${BUS}/${DEV}\n", argv[0]);
      printf("       %s reconnect  /dev/bus/usb/${BUS}/${DEV}\n", argv[0]);
      printf("Disconnect or reconnect USB devices\n");
}

int send_usb_cmd(int fd, int cmd)
{
  struct usbdevfs_ioctl command;

  command.ifno = 0; // interface number, does it matter?

  command.ioctl_code = cmd;
  command.data = NULL;
          
  return ioctl(fd, USBDEVFS_IOCTL, &command);  
}

int main(int argc, char** argv)
{
  if (argc != 3)
    {
      print_usage(argc, argv);
      return EXIT_FAILURE;
    }
  else
    {
      enum Command { CMD_CONNECT,
                     CMD_DISCONNECT,
                     CMD_RECONNECT
      } cmd;
      
      if (strcmp(argv[1], "connect") == 0)
        {
          cmd = CMD_CONNECT;
        }
      else if (strcmp(argv[1], "disconnect") == 0)
        {
          cmd = CMD_DISCONNECT;
        }
      else if (strcmp(argv[1], "reconnect") == 0)
        {
          cmd = CMD_RECONNECT;
        }
      else
        {
          print_usage(argc, argv);
          return EXIT_FAILURE;
        }

      int fd = open(argv[2], O_RDWR);

      if (fd < 0)
        {
          perror(argv[1]);
          return EXIT_FAILURE;
        }
      else
        {
          int ret = 0;
          switch (cmd)
            {
              case CMD_CONNECT:
                ret = send_usb_cmd(fd, USBDEVFS_CONNECT);
                break;
                
              case CMD_DISCONNECT:
                ret = send_usb_cmd(fd, USBDEVFS_DISCONNECT);
                break;

              case CMD_RECONNECT:
                ret = send_usb_cmd(fd, USBDEVFS_DISCONNECT);
                if (ret < 0) goto error_handling;
                sleep(1);
                ret = send_usb_cmd(fd, USBDEVFS_CONNECT);
                break;
                
              default:
                assert(!"Never reached");
            }

        error_handling:
          if (ret < 0)
            {
              printf("%s: Could not issue usb %s on %s: %s\n", argv[0], argv[1], argv[2], strerror(errno));
              close(fd);
              return EXIT_FAILURE;
            }
          else
            {
              printf("%s: %s of %s successful\n", argv[0], argv[1], argv[2]);
              close(fd);
              return EXIT_SUCCESS;
            }
        }
    }
}

/* EOF */
