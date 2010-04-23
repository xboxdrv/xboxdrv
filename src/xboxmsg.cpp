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

#include <assert.h>
#include <boost/format.hpp>
#include <iostream>
#include <algorithm>

#include "helper.hpp"
#include "command_line_options.hpp"
#include "xboxmsg.hpp"

std::string gamepadtype_to_string(const GamepadType& type)
{
  switch (type)
    {
      case GAMEPAD_XBOX360:
        return "xbox360";

      case GAMEPAD_XBOX360_WIRELESS:
        return "xbox360-wireless";

      case GAMEPAD_XBOX:
        return "xbox";

      case GAMEPAD_XBOX_MAT:
        return "xbox-mat";
        
      case GAMEPAD_XBOX360_GUITAR:
        return "xbox360-guitar";

      case GAMEPAD_FIRESTORM:
        return "firestorm";

      case GAMEPAD_FIRESTORM_VSB:
        return "firestorm-vsb";

      case GAMEPAD_SAITEK_P2500:
        return "saitek-p2500";

      default:
        assert(!"Unknown gamepad type supplied");
    }
}

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

      case GAMEPAD_FIRESTORM:
        return out << "Firestorm Dual Power";

      case GAMEPAD_FIRESTORM_VSB:
        return out << "Firestorm Dual Power (vsb)";

      case GAMEPAD_SAITEK_P2500:
        return out << "Saitek P2500";

      default:
        return out << "unknown" << std::endl;
    }
}

std::ostream& operator<<(std::ostream& out, const XboxGenericMsg& msg)
{
  switch (msg.type)
    {
      case XBOX_MSG_XBOX:
        return out << msg.xbox;
        
      case XBOX_MSG_XBOX360:
        return out << msg.xbox360;
      
      case XBOX_MSG_XBOX360_GUITAR:
        return out << msg.guitar;
  
      default:
        return out << "Error: Unhandled XboxGenericMsg type: " << msg.type;
    }
}

std::ostream& operator<<(std::ostream& out, const Xbox360GuitarMsg& msg) 
{
  out << boost::format(" whammy:%6d tilt:%6d | up:%d down:%d left:%d right:%d | back:%d guide:%d start:%d | green:%d red:%d yellow:%d blue:%d orange:%d ")
    % int(msg.whammy)
    % int(msg.tilt)
    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right)
    % int(msg.back)
    % int(msg.guide)
    % int(msg.start)
    % int(msg.green)
    % int(msg.red)
    % int(msg.yellow)
    % int(msg.blue)
    % int(msg.orange);

  if (command_line_options->verbose)
    {
      out << boost::format("| dummy: %d %d %d %d %02hhx %02hhx %04hx %04hx %02x %02x")
        % int(msg.thumb_l)
        % int(msg.thumb_r)
        % int(msg.rb)
        % int(msg.dummy1)

        % int(msg.lt)
        % int(msg.rt)

        % int16_t(msg.x1)
        % int16_t(msg.y1)

        % int(msg.dummy2)
        % int(msg.dummy3);
    }
 
  return out;
}

std::ostream& operator<<(std::ostream& out, const Xbox360Msg& msg) 
{
  out << boost::format("X1:%6d Y1:%6d") 
    % int(msg.x1) % int(msg.y1);

  out << boost::format("  X2:%6d Y2:%6d")
    % int(msg.x2) % int(msg.y2);
                          
  out << boost::format("  du:%d dd:%d dl:%d dr:%d")
    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right);

  out << "  back:" << msg.back;
  out << " guide:" << msg.guide;
  out << " start:" << msg.start;

  out << "  TL:" << msg.thumb_l;
  out << " TR:"  << msg.thumb_r;

  out << "  A:" << msg.a;
  out << " B:"  << msg.b;
  out << " X:"  << msg.x;
  out << " Y:"  << msg.y;

  out << "  LB:" << msg.lb;
  out << " RB:" <<  msg.rb;

  out << boost::format("  LT:%3d RT:%3d")
    % int(msg.lt) % int(msg.rt);

  if (command_line_options->verbose)
    out << " Dummy: " << msg.dummy1 << " " << msg.dummy2 << " " << msg.dummy3;

  return out;
}

std::ostream& operator<<(std::ostream& out, const XboxMsg& msg) 
{
  out << boost::format(" X1:%6d Y1:%6d  X2:%6d Y2:%6d "
                       " du:%d dd:%d dl:%d dr:%d "
                       " start:%d back:%d "
                       " TL:%d TR:%d "
                       " A:%3d B:%3d X:%3d Y:%3d "
                       " black:%3d white:%3d "
                       " LT:%3d RT:%3d ")
    % int(msg.x1) % int(msg.y1)
    % int(msg.x2) % int(msg.y2)

    % int(msg.dpad_up)
    % int(msg.dpad_down)
    % int(msg.dpad_left)
    % int(msg.dpad_right)

    % int(msg.start)
    % int(msg.back)

    % int(msg.thumb_l)
    % int(msg.thumb_r)

    % int(msg.a)
    % int(msg.b)
    % int(msg.x)
    % int(msg.y)

    % int(msg.black)
    % int(msg.white)

    % int(msg.lt) 
    % int(msg.rt);

  if (command_line_options->verbose)
    out << " Dummy: " << msg.dummy;

  return out;
}

int get_button(XboxGenericMsg& msg, XboxButton button)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360:
      case XBOX_MSG_XBOX360_GUITAR:
        switch(button)
          {
            case XBOX_BTN_START:
              return msg.xbox360.start;
            case XBOX_BTN_GUIDE:
              return msg.xbox360.guide;
            case XBOX_BTN_BACK:
              return msg.xbox360.back;

            case XBOX_BTN_GREEN:
            case XBOX_BTN_A:
              return msg.xbox360.a;
            case XBOX_BTN_RED:
            case XBOX_BTN_B:
              return msg.xbox360.b;
            case XBOX_BTN_BLUE:
            case XBOX_BTN_X:
              return msg.xbox360.x;
            case XBOX_BTN_YELLOW:
            case XBOX_BTN_Y:
              return msg.xbox360.y;

            case XBOX_BTN_ORANGE:
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

            case XBOX_BTN_MAX:
            case XBOX_BTN_UNKNOWN:
              return 0;
          }
        break;
        
      case XBOX_MSG_XBOX:
        switch(button)
          {
            case XBOX_BTN_START:
              return msg.xbox.start;
            case XBOX_BTN_GUIDE:
              return 0;
            case XBOX_BTN_BACK:
              return msg.xbox.back;

            case XBOX_BTN_GREEN:
            case XBOX_BTN_A:
              return msg.xbox.a;
            case XBOX_BTN_RED:
            case XBOX_BTN_B:
              return msg.xbox.b;
            case XBOX_BTN_BLUE:
            case XBOX_BTN_X:
              return msg.xbox.x;
            case XBOX_BTN_YELLOW:
            case XBOX_BTN_Y:
              return msg.xbox.y;

            case XBOX_BTN_ORANGE:
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

            case XBOX_BTN_MAX:
            case XBOX_BTN_UNKNOWN:
              return 0;
          }
        break;
    }
  return 0;
}

void set_button(XboxGenericMsg& msg, XboxButton button, int v)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360_GUITAR:
      case XBOX_MSG_XBOX360:
        switch(button)
          {
            case XBOX_BTN_START:
              msg.xbox360.start = v; break;
            case XBOX_BTN_GUIDE:
              msg.xbox360.guide = v; break;
            case XBOX_BTN_BACK:
              msg.xbox360.back = v; break;

            case XBOX_BTN_GREEN:
            case XBOX_BTN_A:
              msg.xbox360.a = v; break;
            case XBOX_BTN_RED:
            case XBOX_BTN_B:
              msg.xbox360.b = v; break;
            case XBOX_BTN_BLUE:
            case XBOX_BTN_X:
              msg.xbox360.x = v; break;
            case XBOX_BTN_YELLOW:
            case XBOX_BTN_Y:
              msg.xbox360.y = v; break;

            case XBOX_BTN_ORANGE:
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

            case XBOX_BTN_MAX:
            case XBOX_BTN_UNKNOWN:
              break;
          }
        break;
        
      case XBOX_MSG_XBOX:
        switch(button)
          {
            case XBOX_BTN_START:
              msg.xbox.start = v; break;
            case XBOX_BTN_GUIDE:
              break;
            case XBOX_BTN_BACK:
              msg.xbox.back = v; break;

            case XBOX_BTN_GREEN:
            case XBOX_BTN_A:
              msg.xbox.a = v; break;
            case XBOX_BTN_RED:
            case XBOX_BTN_B:
              msg.xbox.b = v; break;
            case XBOX_BTN_BLUE:
            case XBOX_BTN_X:
              msg.xbox.x = v; break;
            case XBOX_BTN_YELLOW:
            case XBOX_BTN_Y:
              msg.xbox.y = v; break;

            case XBOX_BTN_ORANGE:
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

            case XBOX_BTN_MAX:
            case XBOX_BTN_UNKNOWN:
              break;
          }
        break;
    }
}

int get_axis(XboxGenericMsg& msg, XboxAxis axis)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360_GUITAR:
      case XBOX_MSG_XBOX360:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
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

      case XBOX_MSG_XBOX:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
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
    }
  return 0;
}

float s16_to_float(int16_t value)
{
  if (value >= 0)
  {
    return static_cast<float>(value) / 32767.0f;
  }
  else
  {
    return static_cast<float>(value) / 32768.0f;
  }
}

float u8_to_float(uint8_t value)
{
  return static_cast<float>(value) / 255.0f;
}

int16_t float_to_s16(float v)
{
  if (v >= 0.0f)
  {
    return static_cast<int16_t>(std::min(1.0f, v) * 32767.0f);
  }
  else
  {
    return static_cast<int16_t>(std::max(-1.0f, v) * 32768.0f);
  }
}

uint8_t float_to_u8(float v)
{
  return static_cast<uint8_t>(Math::clamp(0.0f, 1.0f, v) * 255.0f);
}

float get_axis_float(XboxGenericMsg& msg, XboxAxis axis)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360_GUITAR:
      case XBOX_MSG_XBOX360:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
              return 0.0f;
            case XBOX_AXIS_X1:
              return s16_to_float(msg.xbox360.x1);
            case XBOX_AXIS_Y1:
              return s16_to_float(msg.xbox360.y1);
            case XBOX_AXIS_X2:
              return s16_to_float(msg.xbox360.x2);
            case XBOX_AXIS_Y2:
              return s16_to_float(msg.xbox360.y2);
            case XBOX_AXIS_LT:
              return u8_to_float(msg.xbox360.lt);
            case XBOX_AXIS_RT:
              return u8_to_float(msg.xbox360.rt);
          }
        break;

      case XBOX_MSG_XBOX:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
              return 0.0f;
            case XBOX_AXIS_X1:
              return s16_to_float(msg.xbox.x1);
            case XBOX_AXIS_Y1:
              return s16_to_float(msg.xbox.y1);
            case XBOX_AXIS_X2:
              return s16_to_float(msg.xbox.x2);
            case XBOX_AXIS_Y2:
              return s16_to_float(msg.xbox.y2);
            case XBOX_AXIS_LT:
              return u8_to_float(msg.xbox.lt);
            case XBOX_AXIS_RT:
              return u8_to_float(msg.xbox.rt);
          }
        break;
    }
  return 0;
}

void set_axis_float(XboxGenericMsg& msg, XboxAxis axis, float v)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360_GUITAR:
      case XBOX_MSG_XBOX360:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
              break;
            case XBOX_AXIS_X1:
              msg.xbox360.x1 = float_to_s16(v); break;
            case XBOX_AXIS_Y1:
              msg.xbox360.y1 = float_to_s16(v); break;
            case XBOX_AXIS_X2:
              msg.xbox360.x2 = float_to_s16(v); break;
            case XBOX_AXIS_Y2:
              msg.xbox360.y2 = float_to_s16(v); break;
            case XBOX_AXIS_LT:
              msg.xbox360.lt = float_to_u8(v); break;
            case XBOX_AXIS_RT:
              msg.xbox360.rt = float_to_u8(v); break;
          }
        break;

      case XBOX_MSG_XBOX:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
              break;
            case XBOX_AXIS_X1:
              msg.xbox.x1 = float_to_s16(v); break;
            case XBOX_AXIS_Y1:
              msg.xbox.y1 = float_to_s16(v); break;
            case XBOX_AXIS_X2:
              msg.xbox.x2 = float_to_s16(v); break;
            case XBOX_AXIS_Y2:
              msg.xbox.y2 = float_to_s16(v); break;
            case XBOX_AXIS_LT:
              msg.xbox.lt = float_to_u8(v); break;
            case XBOX_AXIS_RT:
              msg.xbox.rt = float_to_u8(v); break;
          }
        break;
    }
}

void set_axis(XboxGenericMsg& msg, XboxAxis axis, int v)
{
  switch(msg.type)
    {
      case XBOX_MSG_XBOX360_GUITAR:
      case XBOX_MSG_XBOX360:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
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

      case XBOX_MSG_XBOX:
        switch(axis)
          {
            case XBOX_AXIS_MAX:
            case XBOX_AXIS_UNKNOWN:
            case XBOX_AXIS_DPAD_X:
            case XBOX_AXIS_DPAD_Y:
            case XBOX_AXIS_TRIGGER:
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

  else if (str == "a" || str == "1")
    return XBOX_BTN_A;
  else if (str == "b" || str == "2")
    return XBOX_BTN_B;
  else if (str == "x" || str == "3")
    return XBOX_BTN_X;
  else if (str == "y" || str == "4")
    return XBOX_BTN_Y;

  else if (str == "green")
    return XBOX_BTN_GREEN;
  else if (str == "red")
    return XBOX_BTN_RED;
  else if (str == "blue")
    return XBOX_BTN_BLUE;
  else if (str == "yellow")
    return XBOX_BTN_YELLOW;
  else if (str == "orange")
    return XBOX_BTN_ORANGE;

  else if (str == "black")
    return XBOX_BTN_BLACK;
  else if (str == "white")
    return XBOX_BTN_WHITE;

  else if (str == "lb" || str == "5")
    return XBOX_BTN_LB;
  else if (str == "rb" || str == "6")
    return XBOX_BTN_RB;

  else if (str == "lt" || str == "7")
    return XBOX_BTN_LT;
  else if (str == "rt" || str == "8")
    return XBOX_BTN_RT;

  else if (str == "tl")
    return XBOX_BTN_THUMB_L;
  else if (str == "tr")
    return XBOX_BTN_THUMB_R;

  else if (str == "du" || str == "up")
    return XBOX_DPAD_UP;
  else if (str == "dd" || str == "down")
    return XBOX_DPAD_DOWN;
  else if (str == "dl" || str == "left")
    return XBOX_DPAD_LEFT;
  else if (str == "dr" || str == "right")
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

  else if (str == "dpad_x")
    return XBOX_AXIS_DPAD_X;
  else if (str == "dpad_y")
    return XBOX_AXIS_DPAD_Y;

  else if (str == "trigger")
    return XBOX_AXIS_TRIGGER;

  else
    return XBOX_AXIS_UNKNOWN;
}

std::string axis2string(XboxAxis axis)
{
  switch(axis)
    {
      case XBOX_AXIS_MAX:
      case XBOX_AXIS_UNKNOWN: return "unknown";

      case XBOX_AXIS_TRIGGER: return "TRIGGER";

      case XBOX_AXIS_DPAD_X: return "DPAD_X";
      case XBOX_AXIS_DPAD_Y: return "DPAD_Y";

      case XBOX_AXIS_X1: return "X1";
      case XBOX_AXIS_Y1: return "Y1";

      case XBOX_AXIS_X2: return "X2";
      case XBOX_AXIS_Y2: return "Y2";

      case XBOX_AXIS_LT: return "LT";
      case XBOX_AXIS_RT: return "RT";
    }
  return "unknown";
}

std::string btn2string(XboxButton btn)
{
  switch (btn)
    {
      case XBOX_BTN_MAX:
      case XBOX_BTN_UNKNOWN: return "unknown";
      case XBOX_BTN_START: return "Start";
      case XBOX_BTN_GUIDE: return "Guide";
      case XBOX_BTN_BACK: return "Back";

      case XBOX_BTN_A: return "A";
      case XBOX_BTN_B: return "B";
      case XBOX_BTN_X: return "X";
      case XBOX_BTN_Y: return "Y";

      case XBOX_BTN_GREEN:  return "green";
      case XBOX_BTN_RED:    return "red";
      case XBOX_BTN_BLUE:   return "blue";
      case XBOX_BTN_YELLOW: return "yellow";
      case XBOX_BTN_ORANGE: return "orange";

      case XBOX_BTN_WHITE: return "White";
      case XBOX_BTN_BLACK: return "Black";

      case XBOX_BTN_LB: return "LB";
      case XBOX_BTN_RB: return "RB";

      case XBOX_BTN_LT: return "LT";
      case XBOX_BTN_RT: return "RT";

      case XBOX_BTN_THUMB_L: return "TL";
      case XBOX_BTN_THUMB_R: return "TR";

      case XBOX_DPAD_UP:    return "DPAD_UP";
      case XBOX_DPAD_DOWN:  return "DPAD_DOWN";
      case XBOX_DPAD_LEFT:  return "DPAD_LEFT";
      case XBOX_DPAD_RIGHT: return "DPAD_RIGHT";
    }
  return "unknown";
}
  
/* EOF */
