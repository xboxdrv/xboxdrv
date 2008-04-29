/*
 * $Id: evtest.c,v 1.1 2004/01/17 16:59:46 js Exp $
 *
 *  Copyright (c) 1999-2000 Vojtech Pavlik
 *
 *  Event device test program
 *
 *  29.Apr 2008: --list option added by Ingo Ruhnke <grumbel@gmx.de>
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */

#include <linux/input.h>

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char *events[EV_MAX + 1] = { "Reset", "Key", "Relative", "Absolute", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, "LED", "Sound", NULL, "Repeat", "ForceFeedback", NULL, "ForceFeedbackStatus"};
char *keys[KEY_MAX + 1] = { "Reserved", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equal", "Backspace",
"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "LeftBrace", "RightBrace", "Enter", "LeftControl", "A", "S", "D", "F", "G",
"H", "J", "K", "L", "Semicolon", "Apostrophe", "Grave", "LeftShift", "BackSlash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Dot",
"Slash", "RightShift", "KPAsterisk", "LeftAlt", "Space", "CapsLock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
"NumLock", "ScrollLock", "KP7", "KP8", "KP9", "KPMinus", "KP4", "KP5", "KP6", "KPPlus", "KP1", "KP2", "KP3", "KP0", "KPDot", "103rd",
"F13", "102nd", "F11", "F12", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "KPEnter", "RightCtrl", "KPSlash", "SysRq",
"RightAlt", "LineFeed", "Home", "Up", "PageUp", "Left", "Right", "End", "Down", "PageDown", "Insert", "Delete", "Macro", "Mute",
"VolumeDown", "VolumeUp", "Power", "KPEqual", "KPPlusMinus", "Pause", "F21", "F22", "F23", "F24", "KPComma", "LeftMeta", "RightMeta",
"Compose", "Stop", "Again", "Props", "Undo", "Front", "Copy", "Open", "Paste", "Find", "Cut", "Help", "Menu", "Calc", "Setup",
"Sleep", "WakeUp", "File", "SendFile", "DeleteFile", "X-fer", "Prog1", "Prog2", "WWW", "MSDOS", "Coffee", "Direction",
"CycleWindows", "Mail", "Bookmarks", "Computer", "Back", "Forward", "CloseCD", "EjectCD", "EjectCloseCD", "NextSong", "PlayPause",
"PreviousSong", "StopCD", "Record", "Rewind", "Phone", "ISOKey", "Config", "HomePage", "Refresh", "Exit", "Move", "Edit", "ScrollUp",
"ScrollDown", "KPLeftParenthesis", "KPRightParenthesis",
"International1", "International2", "International3", "International4", "International5",
"International6", "International7", "International8", "International9",
"Language1", "Language2", "Language3", "Language4", "Language5", "Language6", "Language7", "Language8", "Language9",
NULL, 
"PlayCD", "PauseCD", "Prog3", "Prog4", "Suspend", "Close",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Btn0", "Btn1", "Btn2", "Btn3", "Btn4", "Btn5", "Btn6", "Btn7", "Btn8", "Btn9",
NULL, NULL,  NULL, NULL, NULL, NULL,
"LeftBtn", "RightBtn", "MiddleBtn", "SideBtn", "ExtraBtn", "ForwardBtn", "BackBtn",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Trigger", "ThumbBtn", "ThumbBtn2", "TopBtn", "TopBtn2", "PinkieBtn",
"BaseBtn", "BaseBtn2", "BaseBtn3", "BaseBtn4", "BaseBtn5", "BaseBtn6",
NULL, NULL, NULL, "BtnDead",
"BtnA", "BtnB", "BtnC", "BtnX", "BtnY", "BtnZ", "BtnTL", "BtnTR", "BtnTL2", "BtnTR2", "BtnSelect", "BtnStart", "BtnMode",
"BtnThumbL", "BtnThumbR", NULL,
"ToolPen", "ToolRubber", "ToolBrush", "ToolPencil", "ToolAirbrush", "ToolFinger", "ToolMouse", "ToolLens", NULL, NULL,
"Touch", "Stylus", "Stylus2", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
"Ok", "Select", "Goto", "Clear", "Power2", "Option", "Info", "Time", "Vendor",
"Archive", "Program", "Channel", "Favorites", "EPG", "PVR", "MHP", "Language",
"Title", "Subtitle", "Angle", "Zoom", "Mode", "Keyboard", "Screen", "PC", "TV",
"TV2", "VCR", "VCR2", "Sat", "Sat2", "CD", "Tape", "Radio", "Tuner", "Player", 
"Text", "DVD", "Aux", "MP3", "Audio", "Video", "Directory", "List", "Memo",
"Calendar", "Red", "Green", "Yellow", "Blue", "ChannelUp", "ChannelDown", 
"First", "Last", "AB", "Play", "Restart", "Slow", "Shuffle", "FastForward", 
"Previous", "Next", "Digits", "Teen", "Twen", "Break" };

char *absval[5] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat " };
char *relatives[REL_MAX + 1] = { "X", "Y", "Z", NULL, NULL, NULL, "HWheel", "Dial", "Wheel" };
char *absolutes[ABS_MAX + 1] = { "X", "Y", "Z", "Rx", "Ry", "Rz", "Throttle", "Rudder", "Wheel", "Gas", "Brake",
NULL, NULL, NULL, NULL, NULL,
"Hat0X", "Hat0Y", "Hat1X", "Hat1Y", "Hat2X", "Hat2Y", "Hat3X", "Hat 3Y", "Pressure", "Distance", "XTilt", "YTilt"};
char *leds[LED_MAX + 1] = { "NumLock", "CapsLock", "ScrollLock", "Compose", "Kana", "Sleep", "Suspend", "Mute" };
char *repeats[REP_MAX + 1] = { "Delay", "Period" };
char *sounds[SND_MAX + 1] = { "Bell", "Click" };

char **names[EV_MAX + 1] = { events, keys, relatives, absolutes, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, leds, sounds, NULL, repeats, NULL, NULL, NULL };

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

void evtest_list();
void evtest_info(const char* filename);
void evtest_test(const char* filename);

int main (int argc, char **argv)
{
  if (argc != 2)
    {
      printf("Usage: evtest FILE\n");
      printf("       evtest --list\n");
      exit(1);
    }
  else if (strcmp(argv[1], "--list") == 0)
    {
      evtest_list();
      return 0;
    }
  else
    {
      evtest_test(argv[1]);
      return 0;
    }
}

void evtest_info(const char* filename)
{
  int fd;
  unsigned short id[4];
  char name[256] = "Unknown";

  if ((fd = open(filename, O_RDONLY)) < 0) 
    {
      perror(filename);
    }
  else
    {
      int version;
      if (ioctl(fd, EVIOCGVERSION, &version)) {
        perror("evtest: can't get version");
        exit(1);
      }

      printf("Input driver version is %d.%d.%d\n",
             version >> 16, (version >> 8) & 0xff, version & 0xff);

      ioctl(fd, EVIOCGID, id);
      printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
             id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

      ioctl(fd, EVIOCGNAME(sizeof(name)), name);
      printf("Input device name: \"%s\"\n", name);

      unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

      memset(bit, 0, sizeof(bit));
      ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
      printf("Supported events:\n");

      for (int i = 0; i < EV_MAX; i++)
        if (test_bit(i, bit[0])) {
          printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
          ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
          for (int j = 0; j < KEY_MAX; j++) 
            if (test_bit(j, bit[i])) {
              printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
              if (i == EV_ABS) {
                int abs[5];
                ioctl(fd, EVIOCGABS(j), abs);
                for (int k = 0; k < 5; k++)
                  if ((k < 3) || abs[k])
                    printf("      %s %6d\n", absval[k], abs[k]);
              }
            }
        }
      putchar('\n');
    }
}

void evtest_list()
{
  char* pathname = "/dev/input";
  DIR* dir = opendir(pathname);
  if (!dir)
    {
      perror(pathname);
    }
  else
    {
      struct dirent* el = 0;
      
      while ((el = readdir(dir)) != 0)
        {
          if (strncmp("event", el->d_name, 5) == 0)
            { // FIXME: This doesn't lead to alphabetic order
              int len = strlen(pathname) + 1 + strlen(el->d_name) + 1;
              char filename[len];
              filename[0] = '\0';
              strcat(filename, pathname);
              strcat(filename, "/");
              strcat(filename, el->d_name);

              evtest_info(filename);
            }
        }

      closedir(dir);
    }  
}

void evtest_test(const char* filename)
{
  int fd, rd, i, j, k;
  struct input_event ev[64];
  int version;
  unsigned short id[4];
  unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
  char name[256] = "Unknown";
  int abs[5];

  if ((fd = open(filename, O_RDONLY)) < 0) {
    perror("evtest");
    exit(1);
  }

  if (ioctl(fd, EVIOCGVERSION, &version)) {
    perror("evtest: can't get version");
    exit(1);
  }

  printf("Input driver version is %d.%d.%d\n",
         version >> 16, (version >> 8) & 0xff, version & 0xff);

  ioctl(fd, EVIOCGID, id);
  printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
         id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

  ioctl(fd, EVIOCGNAME(sizeof(name)), name);
  printf("Input device name: \"%s\"\n", name);

  memset(bit, 0, sizeof(bit));
  ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
  printf("Supported events:\n");

  for (i = 0; i < EV_MAX; i++)
    if (test_bit(i, bit[0])) {
      printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
      ioctl(fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
      for (j = 0; j < KEY_MAX; j++) 
        if (test_bit(j, bit[i])) {
          printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
          if (i == EV_ABS) {
            ioctl(fd, EVIOCGABS(j), abs);
            for (k = 0; k < 5; k++)
              if ((k < 3) || abs[k])
                printf("      %s %6d\n", absval[k], abs[k]);
          }
        }
    }
		

  printf("Testing ... (interrupt to exit)\n");

  while (1) {
    rd = read(fd, ev, sizeof(struct input_event) * 64);

    if (rd < (int) sizeof(struct input_event)) {
      printf("yyy\n");
      perror("\nevtest: error reading");
      exit (1);
    }

    for (i = 0; i < rd / sizeof(struct input_event); i++)
      printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
             ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
             events[ev[i].type] ? events[ev[i].type] : "?",
             ev[i].code,
             names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
             ev[i].value);

  }
}

