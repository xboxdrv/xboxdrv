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

#include <signal.h>
#include <boost/format.hpp>
#include <usb.h>
#include <unistd.h>
#include <iostream>
#include "uinput.hpp"
#include "xboxmsg.hpp"
#include "xbox_controller.hpp"
#include "xbox360_controller.hpp"
#include "xbox360_wireless_controller.hpp"
#include "helper.hpp"
#include "xbox_generic_controller.hpp"

#include "xboxdrv.hpp"

// Some ugly global variables, needed for sigint catching
bool global_exit_xboxdrv = false;
XboxGenericController* global_controller = 0;

XPadDevice xpad_devices[] = {
  // Evil?! Anymore info we could use to identify the devices?
  // { GAMEPAD_XBOX,             0x0000, 0x0000, "Generic X-Box pad" },
  // { GAMEPAD_XBOX,             0xffff, 0xffff, "Chinese-made Xbox Controller" },

  // These should work
  { GAMEPAD_XBOX,             0x045e, 0x0202, "Microsoft X-Box pad v1 (US)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft X-Box pad (Japan)" },
  { GAMEPAD_XBOX,             0x045e, 0x0285, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0287, "Microsoft Xbox Controller S" },
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft X-Box pad v2 (US)" }, // duplicate
  { GAMEPAD_XBOX,             0x045e, 0x0289, "Microsoft Xbox Controller S" }, // duplicate
  { GAMEPAD_XBOX,             0x046d, 0xca84, "Logitech Xbox Cordless Controller" },
  { GAMEPAD_XBOX,             0x046d, 0xca88, "Logitech Compact Controller for Xbox" },
  { GAMEPAD_XBOX,             0x05fd, 0x1007, "Mad Catz Controller (unverified)" },
  { GAMEPAD_XBOX,             0x05fd, 0x107a, "InterAct 'PowerPad Pro' X-Box pad (Germany)" },
  { GAMEPAD_XBOX,             0x0738, 0x4516, "Mad Catz Control Pad" },
  { GAMEPAD_XBOX,             0x0738, 0x4522, "Mad Catz LumiCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4526, "Mad Catz Control Pad Pro" },
  { GAMEPAD_XBOX,             0x0738, 0x4536, "Mad Catz MicroCON" },
  { GAMEPAD_XBOX,             0x0738, 0x4556, "Mad Catz Lynx Wireless Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8802, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x8810, "Zeroplus Xbox Controller" },
  { GAMEPAD_XBOX,             0x0c12, 0x9902, "HAMA VibraX - *FAULTY HARDWARE*" },
  { GAMEPAD_XBOX,             0x0e4c, 0x1097, "Radica Gamester Controller" },
  { GAMEPAD_XBOX,             0x0e4c, 0x2390, "Radica Games Jtech Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0003, "Logic3 Freebird wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0005, "Eclipse wireless Controller" },
  { GAMEPAD_XBOX,             0x0e6f, 0x0006, "Edge wireless Controller" },
  { GAMEPAD_XBOX,             0x0e8f, 0x0201, "SmartJoy Frag Xpad/PS2 adaptor" },
  { GAMEPAD_XBOX,             0x0f30, 0x0202, "Joytech Advanced Controller" },
  { GAMEPAD_XBOX,             0x0f30, 0x8888, "BigBen XBMiniPad Controller" },
  { GAMEPAD_XBOX,             0x102c, 0xff0c, "Joytech Wireless Advanced Controller" },
  { GAMEPAD_XBOX,             0x044f, 0x0f07, "Thrustmaster, Inc. Controller" },
  { GAMEPAD_XBOX360,          0x045e, 0x028e, "Microsoft Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x0738, 0x4716, "Mad Catz Xbox 360 Controller" },
  { GAMEPAD_XBOX360,          0x162e, 0xbeef, "Joytech Neo-Se Take2" },
  { GAMEPAD_XBOX360_GUITAR,   0x1430, 0x4748, "RedOctane Guitar Hero X-plorer" },

  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0291, "Microsoft Xbox 360 Wireless Controller" },
  { GAMEPAD_XBOX360_WIRELESS, 0x045e, 0x0719, "Microsoft Xbox 360 Wireless Controller (PC)" },

  { GAMEPAD_XBOX_MAT,         0x0738, 0x4540, "Mad Catz Beat Pad" },
  { GAMEPAD_XBOX_MAT,         0x0738, 0x6040, "Mad Catz Beat Pad Pro" },
  { GAMEPAD_XBOX_MAT,         0x0c12, 0x8809, "RedOctane Xbox Dance Pad" },
  { GAMEPAD_XBOX_MAT,         0x12ab, 0x8809, "Xbox DDR dancepad" },
  { GAMEPAD_XBOX_MAT,         0x1430, 0x8888, "TX6500+ Dance Pad (first generation)" },
};

const int xpad_devices_count = sizeof(xpad_devices)/sizeof(XPadDevice);

XboxButton string2btn(const std::string& str_);
XboxAxis  string2axis(const std::string& str_);

std::ostream& operator<<(std::ostream& out, const GamepadType& type) 
{
  switch (type)
    {
      case GAMEPAD_XBOX360:
        return out << "Xbox360";

      case GAMEPAD_XBOX360_WIRELESS:
        return out << "Xbox360 (wireless)";

      case GAMEPAD_XBOX:
        return out << "Xbox Classic";

      case GAMEPAD_XBOX_MAT:
        return out << "Xbox Dancepad";
        
      case GAMEPAD_XBOX360_GUITAR:
        return out << "Xbox360 Guitar";

      default:
        return out << "unknown" << std::endl;
    }
}

int get_button(XboxGenericMsg& msg, XboxButton button)
{
  switch(msg.type)
    {
      case GAMEPAD_XBOX360_GUITAR:
      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        switch(button)
          {
            case XBOX_BTN_START:
              return msg.xbox360.start;
            case XBOX_BTN_GUIDE:
              return msg.xbox360.guide;
            case XBOX_BTN_BACK:
              return msg.xbox360.back;

            case XBOX_BTN_A:
              return msg.xbox360.a;
            case XBOX_BTN_B:
              return msg.xbox360.b;
            case XBOX_BTN_X:
              return msg.xbox360.x;
            case XBOX_BTN_Y:
              return msg.xbox360.y;

            case XBOX_BTN_LB:
            case XBOX_BTN_WHITE:
              return msg.xbox360.lb;
            case XBOX_BTN_RB:
            case XBOX_BTN_BLACK:
              return msg.xbox360.rb;

            case XBOX_BTN_LT:
              return msg.xbox360.lt;
            case XBOX_BTN_RT:
              return msg.xbox360.rt;

            case XBOX_BTN_THUMB_L:
              return msg.xbox360.thumb_l;
            case XBOX_BTN_THUMB_R:
              return msg.xbox360.thumb_r;

            case XBOX_DPAD_UP:
              return msg.xbox360.dpad_up;
            case XBOX_DPAD_DOWN:
              return msg.xbox360.dpad_down;
            case XBOX_DPAD_LEFT:
              return msg.xbox360.dpad_left;
            case XBOX_DPAD_RIGHT:
              return msg.xbox360.dpad_right;

            case XBOX_BTN_UNKNOWN:
              return 0;
          }
        break;
        
      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        switch(button)
          {
            case XBOX_BTN_START:
              return msg.xbox.start;
            case XBOX_BTN_GUIDE:
              return 0;
            case XBOX_BTN_BACK:
              return msg.xbox.back;

            case XBOX_BTN_A:
              return msg.xbox.a;
            case XBOX_BTN_B:
              return msg.xbox.b;
            case XBOX_BTN_X:
              return msg.xbox.x;
            case XBOX_BTN_Y:
              return msg.xbox.y;

            case XBOX_BTN_LB:
            case XBOX_BTN_WHITE:
              return msg.xbox.white;
            case XBOX_BTN_RB:
            case XBOX_BTN_BLACK:
              return msg.xbox.black;

            case XBOX_BTN_LT:
              return msg.xbox.lt;
            case XBOX_BTN_RT:
              return msg.xbox.rt;

            case XBOX_BTN_THUMB_L:
              return msg.xbox.thumb_l;
            case XBOX_BTN_THUMB_R:
              return msg.xbox.thumb_r;

            case XBOX_DPAD_UP:
              return msg.xbox.dpad_up;
            case XBOX_DPAD_DOWN:
              return msg.xbox.dpad_down;
            case XBOX_DPAD_LEFT:
              return msg.xbox.dpad_left;
            case XBOX_DPAD_RIGHT:
              return msg.xbox.dpad_right;

            case XBOX_BTN_UNKNOWN:
              return 0;
          }
        break;

      case GAMEPAD_UNKNOWN:
        break;
    }
  return 0;
}

void set_button(XboxGenericMsg& msg, XboxButton button, int v)
{
  switch(msg.type)
    {
      case GAMEPAD_XBOX360_GUITAR:
      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        switch(button)
          {
            case XBOX_BTN_START:
              msg.xbox360.start = v; break;
            case XBOX_BTN_GUIDE:
              msg.xbox360.guide = v; break;
            case XBOX_BTN_BACK:
              msg.xbox360.back = v; break;

            case XBOX_BTN_A:
              msg.xbox360.a = v; break;
            case XBOX_BTN_B:
              msg.xbox360.b = v; break;
            case XBOX_BTN_X:
              msg.xbox360.x = v; break;
            case XBOX_BTN_Y:
              msg.xbox360.y = v; break;

            case XBOX_BTN_LB:
            case XBOX_BTN_WHITE:
              msg.xbox360.lb = v; break;
            case XBOX_BTN_RB:
            case XBOX_BTN_BLACK:
              msg.xbox360.rb = v; break;

            case XBOX_BTN_LT:
              msg.xbox360.lt = v; break;
            case XBOX_BTN_RT:
              msg.xbox360.rt = v; break;

            case XBOX_BTN_THUMB_L:
              msg.xbox360.thumb_l = v; break;
            case XBOX_BTN_THUMB_R:
              msg.xbox360.thumb_r = v; break;

            case XBOX_DPAD_UP:
              msg.xbox360.dpad_up = v; break;
            case XBOX_DPAD_DOWN:
              msg.xbox360.dpad_down = v; break;
            case XBOX_DPAD_LEFT:
              msg.xbox360.dpad_left = v; break;
            case XBOX_DPAD_RIGHT:
              msg.xbox360.dpad_right = v; break;

            case XBOX_BTN_UNKNOWN:
              break;
          }
        break;
        
      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        switch(button)
          {
            case XBOX_BTN_START:
              msg.xbox.start = v; break;
            case XBOX_BTN_GUIDE:
              break;
            case XBOX_BTN_BACK:
              msg.xbox.back = v; break;

            case XBOX_BTN_A:
              msg.xbox.a = v; break;
            case XBOX_BTN_B:
              msg.xbox.b = v; break;
            case XBOX_BTN_X:
              msg.xbox.x = v; break;
            case XBOX_BTN_Y:
              msg.xbox.y = v; break;

            case XBOX_BTN_LB:
            case XBOX_BTN_WHITE:
              msg.xbox.white = v; break;
            case XBOX_BTN_RB:
            case XBOX_BTN_BLACK:
              msg.xbox.black = v; break;

            case XBOX_BTN_LT:
              msg.xbox.lt = v; break;
            case XBOX_BTN_RT:
              msg.xbox.rt = v; break;

            case XBOX_BTN_THUMB_L:
              msg.xbox.thumb_l = v; break;
            case XBOX_BTN_THUMB_R:
              msg.xbox.thumb_r = v; break;

            case XBOX_DPAD_UP:
              msg.xbox.dpad_up = v; break;
            case XBOX_DPAD_DOWN:
              msg.xbox.dpad_down = v; break;
            case XBOX_DPAD_LEFT:
              msg.xbox.dpad_left = v; break;
            case XBOX_DPAD_RIGHT:
              msg.xbox.dpad_right = v; break;

            case XBOX_BTN_UNKNOWN:
              break;
          }
        break;

      case GAMEPAD_UNKNOWN:
        break;
    }
}

int get_axis(XboxGenericMsg& msg, XboxAxis axis)
{
  switch(msg.type)
    {
      case GAMEPAD_XBOX360_GUITAR:
      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        switch(axis)
          {
            case XBOX_AXIS_UNKNOWN:
              return 0;
            case XBOX_AXIS_X1:
              return msg.xbox360.x1;
            case XBOX_AXIS_Y1:
              return msg.xbox360.y1;
            case XBOX_AXIS_X2:
              return msg.xbox360.x2;
            case XBOX_AXIS_Y2:
              return msg.xbox360.y2;
            case XBOX_AXIS_LT:
              return msg.xbox360.lt;
            case XBOX_AXIS_RT:
              return msg.xbox360.rt;
          }
        break;

      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        switch(axis)
          {
            case XBOX_AXIS_UNKNOWN:
              return 0;
            case XBOX_AXIS_X1:
              return msg.xbox.x1;
            case XBOX_AXIS_Y1:
              return msg.xbox.y1;
            case XBOX_AXIS_X2:
              return msg.xbox.x2;
            case XBOX_AXIS_Y2:
              return msg.xbox.y2;
            case XBOX_AXIS_LT:
              return msg.xbox.lt;
            case XBOX_AXIS_RT:
              return msg.xbox.rt;
          }
        break;

      case GAMEPAD_UNKNOWN:
        break;
    }
  return 0;
}

void set_axis(XboxGenericMsg& msg, XboxAxis axis, int v)
{
  switch(msg.type)
    {
      case GAMEPAD_XBOX360_GUITAR:
      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        switch(axis)
          {
            case XBOX_AXIS_UNKNOWN:
              break;
            case XBOX_AXIS_X1:
              msg.xbox360.x1 = v; break;
            case XBOX_AXIS_Y1:
              msg.xbox360.y1 = v; break;
            case XBOX_AXIS_X2:
              msg.xbox360.x2 = v; break;
            case XBOX_AXIS_Y2:
              msg.xbox360.y2 = v; break;
            case XBOX_AXIS_LT:
              msg.xbox360.lt = v; break;
            case XBOX_AXIS_RT:
              msg.xbox360.rt = v; break;
          }
        break;

      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        switch(axis)
          {
            case XBOX_AXIS_UNKNOWN:
              break;
            case XBOX_AXIS_X1:
              msg.xbox.x1 = v; break;
            case XBOX_AXIS_Y1:
              msg.xbox.y1 = v; break;
            case XBOX_AXIS_X2:
              msg.xbox.x2 = v; break;
            case XBOX_AXIS_Y2:
              msg.xbox.y2 = v; break;
            case XBOX_AXIS_LT:
              msg.xbox.lt = v; break;
            case XBOX_AXIS_RT:
              msg.xbox.rt = v; break;
          }
        break;

      case GAMEPAD_UNKNOWN:
        break;
    }
}

void apply_button_map(XboxGenericMsg& msg, std::vector<ButtonMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<ButtonMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->lhs, 0);

  for(std::vector<ButtonMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->rhs, get_button(msg, i->lhs) || get_button(newmsg, i->rhs));

  msg = newmsg;  
}

void apply_axis_map(XboxGenericMsg& msg, std::vector<AxisMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<AxisMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      set_axis(newmsg, i->lhs, 0);
    }

  for(std::vector<AxisMapping>::iterator i = lst.begin(); i != lst.end(); ++i)
    {
      int lhs  = get_axis(msg,    i->lhs);
      int nrhs = get_axis(newmsg, i->rhs);

      if (i->invert)
        {
          if (i->lhs == XBOX_AXIS_LT ||
              i->lhs == XBOX_AXIS_RT)
            {
              lhs = 255 - lhs;
            }
          else
            {
              lhs = -lhs;
            }
        }

      set_axis(newmsg, i->rhs, std::max(std::min(nrhs + lhs, 32767), -32768));
    }
  msg = newmsg;
}

ButtonMapping string2buttonmapping(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == '=')
        {
          ButtonMapping mapping;
          mapping.lhs = string2btn(std::string(str.begin(), i));
          mapping.rhs = string2btn(std::string(i+1, str.end()));
          
          if (mapping.lhs == XBOX_BTN_UNKNOWN ||
              mapping.rhs == XBOX_BTN_UNKNOWN)
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to button mapping");

          return mapping;
        }
    }
  throw std::runtime_error("Couldn't convert string \"" + str + "\" to button mapping");
}

void string2buttonmap(const std::string& str, std::vector<ButtonMapping>& lst)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == ',')
        {
          if (i != start)
            {
              ButtonMapping mapping = string2buttonmapping(std::string(start, i));
              lst.push_back(mapping);
            }
          start = i+1;
        }
    }
  if (start != str.end())
    {
      ButtonMapping mapping = string2buttonmapping(std::string(start, str.end()));
      lst.push_back(mapping);
    }
}

AxisMapping string2axismapping(const std::string& str)
{
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == '=')
        {
          AxisMapping mapping;

          std::string lhs(str.begin(), i);
          std::string rhs(i+1, str.end());

          if (lhs.empty() || rhs.empty())
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");

          if (lhs[0] == '-')
            {
              mapping.invert = true;
              mapping.lhs = string2axis(lhs.substr(1));
            }
          else
            {
              mapping.invert = false;
              mapping.lhs = string2axis(lhs);
            }

          mapping.rhs = string2axis(rhs);

          if (mapping.lhs == XBOX_AXIS_UNKNOWN ||
              mapping.rhs == XBOX_AXIS_UNKNOWN)
            throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");

          return mapping;
        }
    }
  throw std::runtime_error("Couldn't convert string \"" + str + "\" to axis mapping");
}

void string2axismap(const std::string& str, std::vector<AxisMapping>& lst)
{
  std::string::const_iterator start = str.begin();
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
    {
      if (*i == ',')
        {
          if (i != start)
            {
              AxisMapping mapping = string2axismapping(std::string(start, i));
              lst.push_back(mapping);
            }
          start = i+1;
        }
    }
  if (start != str.end())
    {
      AxisMapping mapping = string2axismapping(std::string(start, str.end()));
      lst.push_back(mapping);
    }
}

XboxButton string2btn(const std::string& str_)
{
  std::string str = to_lower(str_);

  if (str == "start")
    return XBOX_BTN_START;
  else if (str == "guide")
    return XBOX_BTN_GUIDE;
  else if (str == "back")
    return XBOX_BTN_BACK;

  else if (str == "a")
    return XBOX_BTN_A;
  else if (str == "b")
    return XBOX_BTN_B;
  else if (str == "x")
    return XBOX_BTN_X;
  else if (str == "y")
    return XBOX_BTN_Y;

  else if (str == "black")
    return XBOX_BTN_BLACK;
  else if (str == "white")
    return XBOX_BTN_WHITE;

  else if (str == "lb")
    return XBOX_BTN_LB;
  else if (str == "rb")
    return XBOX_BTN_RB;

  else if (str == "lt")
    return XBOX_BTN_LT;
  else if (str == "rt")
    return XBOX_BTN_RT;

  else if (str == "tl")
    return XBOX_BTN_THUMB_L;
  else if (str == "tr")
    return XBOX_BTN_THUMB_R;

  else if (str == "du")
    return XBOX_DPAD_UP;
  else if (str == "dd")
    return XBOX_DPAD_DOWN;
  else if (str == "dl")
    return XBOX_DPAD_LEFT;
  else if (str == "dr")
    return XBOX_DPAD_RIGHT;

  else
    return XBOX_BTN_UNKNOWN;
}

XboxAxis string2axis(const std::string& str_)
{
  std::string str = to_lower(str_);
  if (str == "x1")
    return XBOX_AXIS_X1;
  else if (str == "y1")
    return XBOX_AXIS_Y1;
  else if (str == "x2")
    return XBOX_AXIS_X2;
  else if (str == "y2")
    return XBOX_AXIS_Y2;
  else if (str == "lt")
    return XBOX_AXIS_LT;
  else if (str == "rt")
    return XBOX_AXIS_RT;
  else
    return XBOX_AXIS_UNKNOWN;
}

void list_controller()
{
  struct usb_bus* busses = usb_get_busses();

  int id = 0;
  std::cout << " id | wid | idVendor | idProduct | Name" << std::endl;
  std::cout << "----+-----+----------+-----------+--------------------------------------" << std::endl;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          for(int i = 0; i < xpad_devices_count; ++i)
            {
              if (dev->descriptor.idVendor  == xpad_devices[i].idVendor &&
                  dev->descriptor.idProduct == xpad_devices[i].idProduct)
                {
                  if (xpad_devices[i].type == GAMEPAD_XBOX360_WIRELESS)
                    {
                      for(int wid = 0; wid < 4; ++wid)
                        {
                          std::cout << boost::format(" %2d |  %2d |   0x%04x |    0x%04x | %s (Port: %s)")
                            % id
                            % wid
                            % int(xpad_devices[i].idVendor)
                            % int(xpad_devices[i].idProduct)
                            % xpad_devices[i].name 
                            % wid
                                    << std::endl;
                        }
                    }
                  else
                    {
                      std::cout << boost::format(" %2d |  %2d |   0x%04x |    0x%04x | %s")
                        % id
                        % 0
                        % int(xpad_devices[i].idVendor)
                        % int(xpad_devices[i].idProduct)
                        % xpad_devices[i].name 
                                << std::endl;
                    }
                  id += 1;
                  break;
                }
            }
        }
    }

  if (id == 0)
    std::cout << "\nNo controller detected" << std::endl; 
}

bool find_controller_by_path(const char* busid, const char* devid,struct usb_device** xbox_device)
{
  struct usb_bus* busses = usb_get_busses();

  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      if (strcmp(bus->dirname, busid) == 0)
        {
          for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
            {
              if (strcmp(dev->filename, devid) == 0)
                {
                  *xbox_device = dev;
                  return true;
                }
            }
        }
    }
  return 0;
}

/** find the number of the next unused /dev/input/jsX device */
int find_jsdev_number()
{
  for(int i = 0; ; ++i)
    {
      char filename1[32];
      char filename2[32];

      sprintf(filename1, "/dev/input/js%d", i);
      sprintf(filename2, "/dev/js%d", i);

      if (access(filename1, F_OK) != 0 && access(filename2, F_OK) != 0)
        return i;
    }
}

/** find the number of the next unused /dev/input/eventX device */
int find_evdev_number()
{
  for(int i = 0; ; ++i)
    {
      char filename[32];

      sprintf(filename, "/dev/input/event%d", i);

      if (access(filename, F_OK) != 0)
        return i;
    }
}


bool find_xbox360_controller(int id, struct usb_device** xbox_device, XPadDevice* type)
{
  struct usb_bus* busses = usb_get_busses();

  int id_count = 0;
  for (struct usb_bus* bus = busses; bus; bus = bus->next)
    {
      for (struct usb_device* dev = bus->devices; dev; dev = dev->next) 
        {
          if (0)
            std::cout << (boost::format("UsbDevice: idVendor: 0x%04x idProduct: 0x%04x")
                          % int(dev->descriptor.idProduct)
                          % int(dev->descriptor.idVendor))
                      << std::endl;

          for(int i = 0; i < xpad_devices_count; ++i)
            {
              if (dev->descriptor.idVendor  == xpad_devices[i].idVendor &&
                  dev->descriptor.idProduct == xpad_devices[i].idProduct)
                {
                  if (id_count == id)
                    {
                      *xbox_device = dev;
                      *type        = xpad_devices[i];
                      return true;
                    }
                  else
                    {
                      id_count += 1;
                      break;
                    }
                }
            }
        }
    }
  return 0;
}

struct CommandLineOptions 
{
  bool silent;
  bool rumble;
  int  led;
  int  rumble_l;
  int  rumble_r;
  int  controller_id;
  int  wireless_id;
  bool instant_exit;
  bool no_uinput;
  GamepadType gamepad_type;
  char busid[4];
  char devid[4];
  uInputCfg uinput_config;
  int deadzone;
  std::vector<ButtonMapping> button_map;
  std::vector<AxisMapping>   axis_map;

  CommandLineOptions() {
    silent   = false;
    rumble   = false;
    led      = -1;
    rumble_l = 0;
    rumble_r = 0;
    controller_id = 0;
    wireless_id   = 0;
    instant_exit = false;
    no_uinput = false;
    gamepad_type = GAMEPAD_UNKNOWN;
    busid[0] = '\0';
    devid[0] = '\0';
    deadzone = 0;
  }
};

void print_command_line_help(int argc, char** argv)
{
  std::cout << "Usage: " << argv[0] << " [OPTION]..." << std::endl;
  std::cout << "Xbox360 USB Gamepad Userspace Driver" << std::endl;
  std::cout << std::endl;
  std::cout << "General Options: " << std::endl;
  std::cout << "  -h, --help               display this help and exit" << std::endl;
  std::cout << "  --help-led               list possible values for the led" << std::endl;
  std::cout << "  --help-devices           list supported devices" << std::endl;
  std::cout << "  -s, --silent             do not display events on console" << std::endl;
  std::cout << "  -i, --id N               use controller with id N (default: 0)" << std::endl;
  std::cout << "  -w, --wid N              use wireless controller with wid N (default: 0)" << std::endl;
  std::cout << "  -L, --list-controller    list available controllers" << std::endl;
  std::cout << "  -R, --test-rumble        map rumbling to LT and RT (for testing only)" << std::endl;
  std::cout << "  --no-uinput              do not try to start uinput event dispatching" << std::endl;
  std::cout << std::endl;
  std::cout << "Device Options: " << std::endl;
  std::cout << "  -d, --device BUS:DEV     Use device BUS:DEV, do not do any scanning" << std::endl;
  std::cout << std::endl;
  std::cout << "Status Options: " << std::endl;
  std::cout << "  -l, --led NUM            set LED status, see --list-led-values (default: 0)" << std::endl;
  std::cout << "  -r, --rumble L,R         set the speed for both rumble motors [0-255] (default: 0,0)" << std::endl;
  std::cout << "  -q, --quit               only set led and rumble status then quit" << std::endl;
  std::cout << std::endl;
  std::cout << "Configuration Options: " << std::endl;
  std::cout << "  --deadzone INT           Threshold under which axis events are ignored (default: 0)" << std::endl;
  std::cout << "  --trigger-as-button      LT and RT send button instead of axis events" << std::endl;
  std::cout << "  --trigger-as-zaxis       Combine LT and RT to form a zaxis instead" << std::endl;
  std::cout << "  --dpad-as-button         DPad sends button instead of axis events" << std::endl;
  std::cout << "  --dpad-only              Both sticks are ignored, only DPad sends out axis events" << std::endl;
  std::cout << "  --type TYPE              Ignore autodetection and enforce controller type\n"
            << "                           (xbox, xbox-mat, xbox360, xbox360-wireless, xbox360-guitar)" << std::endl;
  std::cout << "  -b, --buttonmap MAP      Remap the buttons as specified by MAP" << std::endl;
  std::cout << "  -a, --axismap MAP        Remap the axis as specified by MAP" << std::endl;
  std::cout << std::endl;
  std::cout << "Report bugs to Ingo Ruhnke <grumbel@gmx.de>" << std::endl;
}

void print_led_help()
{
  std::cout << 
    "Possible values for '--led VALUE' are:\n\n"
    "   0: off\n"
    "   1: all blinking\n"
    "   2: 1/top-left blink, then on\n"
    "   3: 2/top-right blink, then on\n"
    "   4: 3/bottom-left blink, then on\n"
    "   5: 4/bottom-right blink, then on\n"
    "   6: 1/top-left on\n"
    "   7: 2/top-right on\n"
    "   8: 3/bottom-left on\n"
    "   9: 4/bottom-right on\n"
    "  10: rotate\n"
    "  11: blink\n"
    "  12: blink slower\n"
    "  13: rotate with two lights\n"
    "  14: blink\n"
    "  15: blink once\n"
            << std::endl;
}

void parse_command_line(int argc, char** argv, CommandLineOptions& opts)
{  
  for(int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-h") == 0 ||
          strcmp(argv[i], "--help") == 0)
        {
          print_command_line_help(argc, argv);
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "-s") == 0 ||
               strcmp(argv[i], "--silent") == 0)
        {
          opts.silent = true;
        }
      else if (strcmp(argv[i], "--test-rumble") == 0 ||
               strcmp(argv[i], "-R") == 0)
        {
          opts.rumble = true;
        }
      else if (strcmp(argv[i], "-r") == 0 ||
               strcmp(argv[i], "--rumble") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (sscanf(argv[i], "%d,%d", &opts.rumble_l, &opts.rumble_r) == 2)
                {
                  opts.rumble_l = std::max(0, std::min(255, opts.rumble_l));
                  opts.rumble_r = std::max(0, std::min(255, opts.rumble_r));
                }
              else
                {
                  std::cout << "Error: " << argv[i-1] << " expected an argument in form INT,INT" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "-q") == 0 ||
               strcmp(argv[i], "--quit") == 0)
        {
          opts.instant_exit = true;
        }
      else if (strcmp(argv[i], "--no-uinput") == 0)
        {
          opts.no_uinput = true;
        }
      else if (strcmp(argv[i], "-t") == 0 ||
               strcmp(argv[i], "--type") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (strcmp(argv[i], "xbox") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX;
                }
              else if (strcmp(argv[i], "xbox-mat") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX_MAT;
                }
              else if (strcmp(argv[i], "xbox360") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360;
                }
              else if (strcmp(argv[i], "xbox360-guitar") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360_GUITAR;
                }
              else if (strcmp(argv[i], "xbox360-wireless") == 0)
                {
                  opts.gamepad_type = GAMEPAD_XBOX360_WIRELESS;
                }
              else
                {
                  std::cout << "Error: unknown type: " << argv[i] << std::endl;
                  std::cout << "Possible types are:" << std::endl;
                  std::cout << " * xbox" << std::endl;
                  std::cout << " * xbox-emat" << std::endl;
                  std::cout << " * xbox360" << std::endl;
                  std::cout << " * xbox360-guitar" << std::endl;
                  std::cout << " * xbox360-wireless" << std::endl;
                  exit(EXIT_FAILURE); 
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-b") == 0 ||
               strcmp(argv[i], "--buttonmap") == 0)
        {
          ++i;
          if (i < argc)
            {
              string2buttonmap(argv[i], opts.button_map);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-a") == 0 ||
               strcmp(argv[i], "--axismap") == 0)
        {
          ++i;
          if (i < argc)
            {
              string2axismap(argv[i], opts.axis_map);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--id") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.controller_id = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-w") == 0 ||
               strcmp(argv[i], "--wid") == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.wireless_id = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp(argv[i], "-l") == 0 ||
               strcmp(argv[i], "--led") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (strcmp(argv[i], "help") == 0)
                {
                  print_led_help();
                  exit(EXIT_SUCCESS);
                }
              else
                {
                  opts.led = atoi(argv[i]);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }
        }
      else if (strcmp("--dpad-only", argv[i]) == 0)
        {
          opts.uinput_config.dpad_only = true;
        }
      else if (strcmp("--dpad-as-button", argv[i]) == 0)
        {
          opts.uinput_config.dpad_as_button = true;
        }
      else if (strcmp("--deadzone", argv[i]) == 0)
        {
          ++i;
          if (i < argc)
            {
              opts.deadzone = atoi(argv[i]);
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an INT argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp("--trigger-as-button", argv[i]) == 0)
        {
          if (opts.uinput_config.trigger_as_zaxis)
            {
              std::cout << "Error: Can't combine --trigger-as-button and --trigger-as-zaxis" << std::endl;
              exit(EXIT_FAILURE);
            }
          else
            {
              opts.uinput_config.trigger_as_button = true;
            }
        }
      else if (strcmp("--trigger-as-zaxis", argv[i]) == 0)
        {
          if (opts.uinput_config.trigger_as_button)
            {
              std::cout << "Error: Can't combine --trigger-as-button and --trigger-as-zaxis" << std::endl;
              exit(EXIT_FAILURE);
            }
          else
            {
              opts.uinput_config.trigger_as_zaxis = true;
            }
        }
      else if (strcmp("--help-led", argv[i]) == 0)
        {
          print_led_help();
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "--device") == 0 ||
               strcmp(argv[i], "-d") == 0)
        {
          ++i;
          if (i < argc)
            {
              if (sscanf(argv[i], "%3s:%3s", opts.busid, opts.devid) == 2)
                {
                  std::cout << "     ***************************************" << std::endl;
                  std::cout << "     *** WARNING *** WARNING *** WARNING ***" << std::endl;
                  std::cout << "     ***************************************" << std::endl;
                  std::cout << "The '--device DEV' option should not be needed for normal use" << std::endl;
                  std::cout << "and might potentially be harmful when used on devices that" << std::endl;
                  std::cout << "are not a gamepad, use at your own risk and ensure that you" << std::endl;
                  std::cout << "are accessing the right device.\n"  << std::endl;
                  std::cout << "If you have multiple gamepads and want to select a differnt" << std::endl;
                  std::cout << "one use the '-id N' option instead.\n" << std::endl;
                  std::cout << "Press Ctrl-c to exit and Enter to continue." << std::endl;
                  getchar();
                }
              else
                {
                  std::cout << "Error: " << argv[i-1] << " expected an argument in form BUS:DEV (i.e. 006:003)" << std::endl;
                  exit(EXIT_FAILURE);
                }
            }
          else
            {
              std::cout << "Error: " << argv[i-1] << " expected an argument" << std::endl;
              exit(EXIT_FAILURE);
            }          
        }
      else if (strcmp(argv[i], "--list-controller") == 0 ||
               strcmp(argv[i], "-L") == 0)
        {
          usb_init();
          usb_find_busses();
          usb_find_devices();

          list_controller();
          exit(EXIT_SUCCESS);
        }
      else if (strcmp(argv[i], "--help-devices") == 0)
        {
          std::cout << " idVendor | idProduct | Name" << std::endl;
          std::cout << "----------+-----------+---------------------------------" << std::endl;
          for(unsigned int i = 0; i < sizeof(xpad_devices)/sizeof(XPadDevice); ++i)
            {
              std::cout << boost::format("   0x%04x |    0x%04x | %s")
                % int(xpad_devices[i].idVendor)
                % int(xpad_devices[i].idProduct)
                % xpad_devices[i].name 
                        << std::endl;
            }
          exit(EXIT_SUCCESS);
        }
      else
        {
          std::cout << "Error: unknown command line option: " << argv[i] << std::endl;
          exit(EXIT_FAILURE);
        }
    }
}

void print_info(struct usb_device* dev,
                const XPadDevice& dev_type,
                const CommandLineOptions& opts)
{
  std::cout << "USB Device:        " << dev->bus->dirname << ":" << dev->filename << std::endl;
  std::cout << "Controller:        " << boost::format("\"%s\" (idVendor: 0x%04x, idProduct: 0x%04x)")
    % dev_type.name % uint16_t(dev->descriptor.idVendor) % uint16_t(dev->descriptor.idProduct) << std::endl;
  if (dev_type.type == GAMEPAD_XBOX360_WIRELESS)
    std::cout << "Wireless Port:     " << opts.wireless_id << std::endl;
  std::cout << "Controller Type:   " << opts.gamepad_type << std::endl;
  std::cout << "Deadzone:          " << opts.deadzone << std::endl;
  std::cout << "Rumble Debug:      " << (opts.rumble ? "on" : "off") << std::endl;
  std::cout << "Rumble Speed:      " << "left: " << opts.rumble_l << " right: " << opts.rumble_r << std::endl;
  if (opts.led == -1)
    std::cout << "LED Status:        " << "auto" << std::endl;
  else
    std::cout << "LED Status:        " << opts.led << std::endl;
  std::cout << "ButtonMap:         ";
  if (opts.button_map.empty())
    {
      std::cout << "none" << std::endl;
    }
  else
    {
      for(std::vector<ButtonMapping>::const_iterator i = opts.button_map.begin(); i != opts.button_map.end(); ++i)
        {
          std::cout << i->lhs << "->" << i->rhs << " ";
        }
      std::cout << std::endl;
    }

  std::cout << "AxisMap:           ";
  if (opts.axis_map.empty())
    {
      std::cout << "none" << std::endl;
    }
  else
    {
      for(std::vector<AxisMapping>::const_iterator i = opts.axis_map.begin(); i != opts.axis_map.end(); ++i)
        {
          std::cout << i->lhs << "->" << i->rhs << " ";
        }
      std::cout << std::endl;
    }
}

void apply_deadzone(XboxGenericMsg& msg, int deadzone)
{
  switch (msg.type)
    {
      case GAMEPAD_XBOX:
      case GAMEPAD_XBOX_MAT:
        if (abs(msg.xbox.x1) < deadzone)
          msg.xbox.x1 = 0;
        if (abs(msg.xbox.y1) < deadzone)
          msg.xbox.y1 = 0;
        if (abs(msg.xbox.x2) < deadzone)
          msg.xbox.x2 = 0;
        if (abs(msg.xbox.y2) < deadzone)
          msg.xbox.y2 = 0;
        break;

      case GAMEPAD_XBOX360:
      case GAMEPAD_XBOX360_WIRELESS:
        if (abs(msg.xbox360.x1) < deadzone)
          msg.xbox360.x1 = 0;
        if (abs(msg.xbox360.y1) < deadzone)
          msg.xbox360.y1 = 0;
        if (abs(msg.xbox360.x2) < deadzone)
          msg.xbox360.x2 = 0;
        if (abs(msg.xbox360.y2) < deadzone)
          msg.xbox360.y2 = 0;      
        break;

      case GAMEPAD_XBOX360_GUITAR:
        // FIXME: any use for deadzone here?
        break;

      default:
        assert(!"apply_deadzone(): Unknown gamepad type");
    }
}

void controller_loop(uInput* uinput, XboxGenericController* controller, CommandLineOptions& opts)
{
  XboxGenericMsg oldmsg;
  memset(&oldmsg, 0, sizeof(oldmsg));
  while(!global_exit_xboxdrv)
    {
      XboxGenericMsg msg;

      if (controller->read(msg))
        {
          apply_deadzone(msg, opts.deadzone);

          if (!opts.button_map.empty())
            apply_button_map(msg, opts.button_map);

          if (!opts.axis_map.empty())
            apply_axis_map(msg,   opts.axis_map);

          if (memcmp(&msg, &oldmsg, sizeof(XboxGenericMsg)))
            { // Only send a new event out if something has changed,
              // this is useful since some controllers send events
              // even if nothing has changed, deadzone can cause this
              // too
              oldmsg = msg;

              if (!opts.silent)
                std::cout << msg << std::endl;

              if (uinput) 
                uinput->send(msg);
                    
              if (opts.rumble)
                {
                  if (opts.gamepad_type == GAMEPAD_XBOX)
                    {
                      controller->set_rumble(msg.xbox.lt, msg.xbox.rt);
                    }
                  else if (opts.gamepad_type == GAMEPAD_XBOX360 ||
                           opts.gamepad_type == GAMEPAD_XBOX360_WIRELESS)
                    {
                      controller->set_rumble(msg.xbox360.lt, msg.xbox360.rt);                      
                    }
                }
            }
        }
    }
}

void find_controller(struct usb_device*& dev,
                     XPadDevice&         dev_type,
                     const CommandLineOptions& opts)
{
  if (opts.busid[0] != '\0' && opts.devid[0] != '\0')
    {
      if (opts.gamepad_type == GAMEPAD_UNKNOWN)
        {
          std::cout << "Error: --device BUS:DEV option must be used in combination with --type TYPE option" << std::endl;
          exit(EXIT_FAILURE);
        }
      else
        {
          if (!find_controller_by_path(opts.busid, opts.devid, &dev))
            {
              std::cout << "Error: couldn't find device " << opts.busid << ":" << opts.devid << std::endl;
              exit(EXIT_FAILURE);
            }
        }
    }
  else
    {
      if (!find_xbox360_controller(opts.controller_id, &dev, &dev_type))
        {
          std::cout << "No Xbox or Xbox360 controller found" << std::endl;
          exit(EXIT_FAILURE);
        }
    }
}

int led_count = 0;

void on_sigint(int)
{
  if (global_exit_xboxdrv)
    {
      std::cout << "Ctrl-c pressed twice, exting hard" << std::endl;
      exit(EXIT_SUCCESS);
    }
  else
    {
      std::cout << "Shutdown initiated" << std::endl;
      global_exit_xboxdrv = true; 
      if (global_controller)
        global_controller->set_led(0);
    }
}

int main(int argc, char** argv)
{
  try 
    {
      signal(SIGINT, on_sigint);

      CommandLineOptions opts;

      parse_command_line(argc, argv, opts);

      usb_init();
      usb_find_busses();
      usb_find_devices();
    
      struct usb_device* dev      = 0;
      XPadDevice         dev_type;
  
      find_controller(dev, dev_type, opts);

      if (!dev)
        {
          std::cout << "No suitable USB device found, abort" << std::endl;
          exit(EXIT_FAILURE);
        }
      else 
        {
          if (opts.gamepad_type != GAMEPAD_UNKNOWN)
            { // Override the default gamepad type when given
              dev_type.type = opts.gamepad_type;
            }
          else
            {
              opts.gamepad_type = dev_type.type;
            }
 
          print_info(dev, dev_type, opts);

          XboxGenericController* controller = 0;

          switch (dev_type.type)
            {
              case GAMEPAD_XBOX:
              case GAMEPAD_XBOX_MAT:
                controller = new XboxController(dev);
                break;

              case GAMEPAD_XBOX360_GUITAR:
                controller = new Xbox360Controller(dev, true);
                break;

              case GAMEPAD_XBOX360:
                controller = new Xbox360Controller(dev, false);
                break;

              case GAMEPAD_XBOX360_WIRELESS:
                controller = new Xbox360WirelessController(dev, opts.wireless_id);
                break;

              default:
                assert(!"Unknown gamepad type");
            }

          global_controller = controller;

          int jsdev_number = find_jsdev_number();
          int evdev_number = find_evdev_number();

          // FIXME: insert /dev/input/jsX detection magic here
          if (opts.led == -1)
            controller->set_led(2 + jsdev_number % 4);
          else
            controller->set_led(opts.led);

          controller->set_rumble(opts.rumble_l, opts.rumble_r);
      
          if (opts.instant_exit)
            {
              usleep(1000);
            }
          else
            {          
              uInput* uinput = 0;
              if (!opts.no_uinput)
                {
                  std::cout << "Starting with uinput" << std::endl;
                  uinput = new uInput(opts.gamepad_type, opts.uinput_config);
                }
              else
                {
                  std::cout << "Starting without uinput" << std::endl;
                }
              std::cout << "\nYour Xbox/Xbox360 controller should now be available as:" << std::endl
                        << "  /dev/input/js" << jsdev_number << std::endl
                        << "  /dev/input/event" << evdev_number << std::endl;
          
              std::cout << "\nPress Ctrl-c to quit\n" << std::endl;
          
              global_exit_xboxdrv = false;
              controller_loop(uinput, controller, opts);
          
              delete controller;
              delete uinput;
          
              std::cout << "Shutdown complete" << std::endl;
            }
        }
    }
  catch(std::exception& err)
    {
      std::cout << "Exception: " << err.what() << std::endl;
    }

  return 0;
}

/* EOF */
