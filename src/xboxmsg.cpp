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

#include "xboxmsg.hpp"

#include <boost/format.hpp>

#include "helper.hpp"
#include "raise_exception.hpp"

int16_t u8_to_s16(uint8_t value)
{
  // FIXME: verify this
  if (value < 128)
  {
    return -32768 + (value * 32768 / 128);
  }
  else
  {
    return (value-128) * 32767 / 127;
  }
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

/**
   input:  [0, 255]
   output: [ -1.0f, 1.0f ] 
*/
float u8_to_float(uint8_t value)
{
  return static_cast<float>(value) / 255.0f * 2.0f - 1.0f;
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

/**
   input:  [ -1.0f, 1.0f ] 
   output: [0, 255]
*/
uint8_t float_to_u8(float v)
{
  return static_cast<uint8_t>(Math::clamp(0.0f, (v + 1.0f) / 2.0f, 1.0f) * 255.0f);
}

XboxGenericMsg::XboxGenericMsg() :
  type(XBOX_MSG_XBOX360),
  xbox360(),
  xbox(),
  ps3usb()
{
}

void
XboxGenericMsg::clear()
{
  memset(&xbox360, 0, sizeof(xbox360));
  memset(&xbox,    0, sizeof(xbox));
  memset(&ps3usb,  0, sizeof(ps3usb));
}

int
XboxGenericMsg::get_button(XboxButton button) const
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(button)
      {
        case XBOX_BTN_START:   return this->xbox360.start;
        case XBOX_BTN_GUIDE:   return this->xbox360.guide;
        case XBOX_BTN_BACK:    return this->xbox360.back;

        case XBOX_BTN_A:       return this->xbox360.a;
        case XBOX_BTN_B:       return this->xbox360.b;
        case XBOX_BTN_X:       return this->xbox360.x;
        case XBOX_BTN_Y:       return this->xbox360.y;

        case XBOX_BTN_LB:      return this->xbox360.lb;
        case XBOX_BTN_RB:      return this->xbox360.rb;

        case XBOX_BTN_LT:      return this->xbox360.lt;
        case XBOX_BTN_RT:      return this->xbox360.rt;

        case XBOX_BTN_THUMB_L: return this->xbox360.thumb_l;
        case XBOX_BTN_THUMB_R: return this->xbox360.thumb_r;

        case XBOX_DPAD_UP:     return this->xbox360.dpad_up;
        case XBOX_DPAD_DOWN:   return this->xbox360.dpad_down;
        case XBOX_DPAD_LEFT:   return this->xbox360.dpad_left;
        case XBOX_DPAD_RIGHT:  return this->xbox360.dpad_right;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN: return 0;
      }
      break;
        
    case XBOX_MSG_XBOX:
      switch(button)
      {
        case XBOX_BTN_START: return this->xbox.start;
        case XBOX_BTN_GUIDE: return 0;
        case XBOX_BTN_BACK:  return this->xbox.back;

        case XBOX_BTN_A:     return this->xbox.a;
        case XBOX_BTN_B:     return this->xbox.b;
        case XBOX_BTN_X:     return this->xbox.x;
        case XBOX_BTN_Y:     return this->xbox.y;

        case XBOX_BTN_LB:    return this->xbox.white;
        case XBOX_BTN_RB:    return this->xbox.black;

        case XBOX_BTN_LT:    return this->xbox.lt;
        case XBOX_BTN_RT:    return this->xbox.rt;

        case XBOX_BTN_THUMB_L: return this->xbox.thumb_l;
        case XBOX_BTN_THUMB_R: return this->xbox.thumb_r;

        case XBOX_DPAD_UP:     return this->xbox.dpad_up;
        case XBOX_DPAD_DOWN:   return this->xbox.dpad_down;
        case XBOX_DPAD_LEFT:   return this->xbox.dpad_left;
        case XBOX_DPAD_RIGHT:  return this->xbox.dpad_right;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN: return 0;
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(button)
      {
        case XBOX_BTN_START:   return this->ps3usb.start;
        case XBOX_BTN_GUIDE:   return this->ps3usb.playstation;
        case XBOX_BTN_BACK:    return this->ps3usb.select;

        case XBOX_BTN_A:       return this->ps3usb.cross;
        case XBOX_BTN_B:       return this->ps3usb.circle;
        case XBOX_BTN_X:       return this->ps3usb.square;
        case XBOX_BTN_Y:       return this->ps3usb.triangle;

        case XBOX_BTN_LB:      return this->ps3usb.l1;
        case XBOX_BTN_RB:      return this->ps3usb.r1;

        case XBOX_BTN_LT:      return this->ps3usb.l2;
        case XBOX_BTN_RT:      return this->ps3usb.r2;

        case XBOX_BTN_THUMB_L: return this->ps3usb.l3;
        case XBOX_BTN_THUMB_R: return this->ps3usb.r3;

        case XBOX_DPAD_UP:     return this->ps3usb.dpad_up;
        case XBOX_DPAD_DOWN:   return this->ps3usb.dpad_down;
        case XBOX_DPAD_LEFT:   return this->ps3usb.dpad_left;
        case XBOX_DPAD_RIGHT:  return this->ps3usb.dpad_right;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN: return 0;
      }
      break;
  }
  return 0;
}

void
XboxGenericMsg::set_button(XboxButton button, bool v)
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(button)
      {
        case XBOX_BTN_START:   this->xbox360.start = v; break;
        case XBOX_BTN_GUIDE:   this->xbox360.guide = v; break;
        case XBOX_BTN_BACK:    this->xbox360.back  = v; break;

        case XBOX_BTN_A:       this->xbox360.a = v; break;
        case XBOX_BTN_B:       this->xbox360.b = v; break;
        case XBOX_BTN_X:       this->xbox360.x = v; break;
        case XBOX_BTN_Y:       this->xbox360.y = v; break;

        case XBOX_BTN_LB:      this->xbox360.lb = v; break;
        case XBOX_BTN_RB:      this->xbox360.rb = v; break;

        case XBOX_BTN_LT:      this->xbox360.lt = v*255; break;
        case XBOX_BTN_RT:      this->xbox360.rt = v*255; break;

        case XBOX_BTN_THUMB_L: this->xbox360.thumb_l = v; break;
        case XBOX_BTN_THUMB_R: this->xbox360.thumb_r = v; break;

        case XBOX_DPAD_UP:     this->xbox360.dpad_up = v; break;
        case XBOX_DPAD_DOWN:   this->xbox360.dpad_down = v; break;
        case XBOX_DPAD_LEFT:   this->xbox360.dpad_left = v; break;
        case XBOX_DPAD_RIGHT:  this->xbox360.dpad_right = v; break;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN: break;
      }
      break;
        
    case XBOX_MSG_XBOX:
      switch(button)
      {
        case XBOX_BTN_START:   this->xbox.start = v; break;
        case XBOX_BTN_GUIDE:   break;
        case XBOX_BTN_BACK:    this->xbox.back = v; break;

        case XBOX_BTN_A:       this->xbox.a = v; break;
        case XBOX_BTN_B:       this->xbox.b = v; break;
        case XBOX_BTN_X:       this->xbox.x = v; break;
        case XBOX_BTN_Y:       this->xbox.y = v; break;

        case XBOX_BTN_LB:      this->xbox.white = v; break;
        case XBOX_BTN_RB:      this->xbox.black = v; break;

        case XBOX_BTN_LT:      this->xbox.lt = v; break;
        case XBOX_BTN_RT:      this->xbox.rt = v; break;

        case XBOX_BTN_THUMB_L: this->xbox.thumb_l = v; break;
        case XBOX_BTN_THUMB_R: this->xbox.thumb_r = v; break;

        case XBOX_DPAD_UP:     this->xbox.dpad_up = v; break;
        case XBOX_DPAD_DOWN:   this->xbox.dpad_down = v; break;
        case XBOX_DPAD_LEFT:   this->xbox.dpad_left = v; break;
        case XBOX_DPAD_RIGHT:  this->xbox.dpad_right = v; break;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN:
          break;
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(button)
      {
        case XBOX_BTN_START:   this->ps3usb.start = v; break;
        case XBOX_BTN_GUIDE:   this->ps3usb.playstation = v; break;
        case XBOX_BTN_BACK:    this->ps3usb.select = v; break;

        case XBOX_BTN_A:       this->ps3usb.cross = v; break;
        case XBOX_BTN_B:       this->ps3usb.circle = v; break;
        case XBOX_BTN_X:       this->ps3usb.square = v; break;
        case XBOX_BTN_Y:       this->ps3usb.triangle = v; break;

        case XBOX_BTN_LB:      this->ps3usb.l1 = v; break;
        case XBOX_BTN_RB:      this->ps3usb.r1 = v; break;

        case XBOX_BTN_LT:      this->ps3usb.l2 = v; break;
        case XBOX_BTN_RT:      this->ps3usb.r2 = v; break;

        case XBOX_BTN_THUMB_L: this->ps3usb.l3 = v; break;
        case XBOX_BTN_THUMB_R: this->ps3usb.r3 = v; break;

        case XBOX_DPAD_UP:     this->ps3usb.dpad_up = v; break;
        case XBOX_DPAD_DOWN:   this->ps3usb.dpad_down = v; break;
        case XBOX_DPAD_LEFT:   this->ps3usb.dpad_left = v; break;
        case XBOX_DPAD_RIGHT:  this->ps3usb.dpad_right = v; break;

        case XBOX_BTN_MAX:
        case XBOX_BTN_UNKNOWN:
          break;
      }
      break;
  }
}

int
XboxGenericMsg::get_axis(XboxAxis axis) const
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->xbox360.dpad_left)
          {
            return -1;
          }
          else if (this->xbox360.dpad_right)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->xbox360.dpad_up)
          {
            return -1;
          }
          else if (this->xbox360.dpad_down)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_TRIGGER:
          return this->xbox360.rt - this->xbox360.lt;

        case XBOX_AXIS_X1: return this->xbox360.x1;
        case XBOX_AXIS_Y1: return this->xbox360.y1;
        case XBOX_AXIS_X2: return this->xbox360.x2;
        case XBOX_AXIS_Y2: return this->xbox360.y2;
        case XBOX_AXIS_LT: return this->xbox360.lt;
        case XBOX_AXIS_RT: return this->xbox360.rt;

        case XBOX_AXIS_A:     return 0;
        case XBOX_AXIS_B:     return 0;
        case XBOX_AXIS_X:     return 0;
        case XBOX_AXIS_Y:     return 0;
        case XBOX_AXIS_BLACK: return 0;
        case XBOX_AXIS_WHITE: return 0;
      }
      break;

    case XBOX_MSG_XBOX:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->xbox.dpad_left)
          {
            return -1;
          }
          else if (this->xbox.dpad_right)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->xbox.dpad_up)
          {
            return -1;
          }
          else if (this->xbox.dpad_down)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_TRIGGER: return this->xbox.rt - this->xbox.lt;

        case XBOX_AXIS_X1: return this->xbox.x1;
        case XBOX_AXIS_Y1: return this->xbox.y1;
        case XBOX_AXIS_X2: return this->xbox.x2;
        case XBOX_AXIS_Y2: return this->xbox.y2;
        case XBOX_AXIS_LT: return this->xbox.lt;
        case XBOX_AXIS_RT: return this->xbox.rt;

        case XBOX_AXIS_A:     return this->xbox.a;
        case XBOX_AXIS_B:     return this->xbox.b;
        case XBOX_AXIS_X:     return this->xbox.x;
        case XBOX_AXIS_Y:     return this->xbox.y;
        case XBOX_AXIS_BLACK: return this->xbox.black;
        case XBOX_AXIS_WHITE: return this->xbox.white;
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->ps3usb.dpad_left)
          {
            return -1;
          }
          else if (this->ps3usb.dpad_right)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->ps3usb.dpad_up)
          {
            return -1;
          }
          else if (this->ps3usb.dpad_down)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_TRIGGER: return this->ps3usb.a_r2 - this->ps3usb.a_l2;

        case XBOX_AXIS_X1: return u8_to_s16(this->ps3usb.x1);
        case XBOX_AXIS_Y1: return u8_to_s16(this->ps3usb.y1);
        case XBOX_AXIS_X2: return u8_to_s16(this->ps3usb.x2);
        case XBOX_AXIS_Y2: return u8_to_s16(this->ps3usb.y2);
        case XBOX_AXIS_LT: return this->ps3usb.a_l2;
        case XBOX_AXIS_RT: return this->ps3usb.a_r2;

        case XBOX_AXIS_A:     return this->ps3usb.a_cross;
        case XBOX_AXIS_B:     return this->ps3usb.a_circle;
        case XBOX_AXIS_X:     return this->ps3usb.a_square;
        case XBOX_AXIS_Y:     return this->ps3usb.a_triangle;
        case XBOX_AXIS_BLACK: return this->ps3usb.a_l1;
        case XBOX_AXIS_WHITE: return this->ps3usb.a_r1;
      }
      break;
  }
  return 0;
}

void
XboxGenericMsg::set_axis(XboxAxis axis, int v)
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_TRIGGER:
          this->xbox360.lt = v < 0 ? v : 0;
          this->xbox360.rt = v > 0 ? v : 0;
          break;

        case XBOX_AXIS_DPAD_X:
          if (v > 0)
          {
            this->xbox360.dpad_left  = false;
            this->xbox360.dpad_right =  true;
          }
          else if (v < 0)
          {
            this->xbox360.dpad_left   = true;
            this->xbox360.dpad_right = false;
          }
          else
          {
            this->xbox360.dpad_left  = false;
            this->xbox360.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0)
          {
            this->xbox360.dpad_up   = false;
            this->xbox360.dpad_down =  true;
          }
          else if (v < 0)
          {
            this->xbox360.dpad_up   = true;
            this->xbox360.dpad_down = false;
          }
          else
          {
            this->xbox360.dpad_up   = false;
            this->xbox360.dpad_down = false;
          }
          break;

        case XBOX_AXIS_X1: this->xbox360.x1 = v; break;
        case XBOX_AXIS_Y1: this->xbox360.y1 = v; break;
        case XBOX_AXIS_X2: this->xbox360.x2 = v; break;
        case XBOX_AXIS_Y2: this->xbox360.y2 = v; break;
        case XBOX_AXIS_LT: this->xbox360.lt = v; break;
        case XBOX_AXIS_RT: this->xbox360.rt = v; break;

        case XBOX_AXIS_A:     break;
        case XBOX_AXIS_B:     break;
        case XBOX_AXIS_X:     break;
        case XBOX_AXIS_Y:     break;
        case XBOX_AXIS_BLACK: break;
        case XBOX_AXIS_WHITE: break;
      }
      break;

    case XBOX_MSG_XBOX:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (v > 0)
          {
            this->xbox.dpad_left  = false;
            this->xbox.dpad_right =  true;
          }
          else if (v < 0)
          {
            this->xbox.dpad_left   = true;
            this->xbox.dpad_right = false;
          }
          else
          {
            this->xbox.dpad_left  = false;
            this->xbox.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0)
          {
            this->xbox.dpad_up   = false;
            this->xbox.dpad_down = true;
          }
          else if (v < 0)
          {
            this->xbox.dpad_up   = true;
            this->xbox.dpad_down = false;
          }
          else
          {
            this->xbox.dpad_up   = false;
            this->xbox.dpad_down = false;
          }
          break;

        case XBOX_AXIS_TRIGGER:
          this->xbox.lt = v < 0 ? v : 0;
          this->xbox.rt = v > 0 ? v : 0;
          break;

        case XBOX_AXIS_X1: this->xbox.x1 = v; break;
        case XBOX_AXIS_Y1: this->xbox.y1 = v; break;
        case XBOX_AXIS_X2: this->xbox.x2 = v; break;
        case XBOX_AXIS_Y2: this->xbox.y2 = v; break;
        case XBOX_AXIS_LT: this->xbox.lt = v; break;
        case XBOX_AXIS_RT: this->xbox.rt = v; break;

        case XBOX_AXIS_A:     this->xbox.a = v; break;
        case XBOX_AXIS_B:     this->xbox.b = v; break;
        case XBOX_AXIS_X:     this->xbox.x = v; break;
        case XBOX_AXIS_Y:     this->xbox.y = v; break;
        case XBOX_AXIS_BLACK: this->xbox.black = v; break;
        case XBOX_AXIS_WHITE: this->xbox.white = v; break;
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_TRIGGER:
          this->ps3usb.a_l2 = v < 0 ? v : 0;
          this->ps3usb.a_r2 = v > 0 ? v : 0;
          break;

        case XBOX_AXIS_DPAD_X:
          if (v > 0)
          {
            this->ps3usb.dpad_left  = false;
            this->ps3usb.dpad_right =  true;
          }
          else if (v < 0)
          {
            this->ps3usb.dpad_left   = true;
            this->ps3usb.dpad_right = false;
          }
          else
          {
            this->ps3usb.dpad_left  = false;
            this->ps3usb.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0)
          {
            this->ps3usb.dpad_up   = false;
            this->ps3usb.dpad_down = true;
          }
          else if (v < 0)
          {
            this->ps3usb.dpad_up   = true;
            this->ps3usb.dpad_down = false;
          }
          else
          {
            this->ps3usb.dpad_down = false;
            this->ps3usb.dpad_up   = false;
          }
          break;

        case XBOX_AXIS_X1: this->ps3usb.x1 = v; break;
        case XBOX_AXIS_Y1: this->ps3usb.y1 = v; break;
        case XBOX_AXIS_X2: this->ps3usb.x2 = v; break;
        case XBOX_AXIS_Y2: this->ps3usb.y2 = v; break;
        case XBOX_AXIS_LT: this->ps3usb.a_l2 = v; break;
        case XBOX_AXIS_RT: this->ps3usb.a_r2 = v; break;

        case XBOX_AXIS_A:     this->ps3usb.a_cross    = v; break;
        case XBOX_AXIS_B:     this->ps3usb.a_circle   = v; break;
        case XBOX_AXIS_X:     this->ps3usb.a_square   = v; break;
        case XBOX_AXIS_Y:     this->ps3usb.a_triangle = v; break;
        case XBOX_AXIS_BLACK: this->ps3usb.a_l1 = v; break;
        case XBOX_AXIS_WHITE: this->ps3usb.a_r1 = v; break;
      }
      break;
  }
}

float
XboxGenericMsg::get_axis_float(XboxAxis axis) const
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->xbox360.dpad_left)
          {
            return -1;
          }
          else if (this->xbox360.dpad_right)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->xbox360.dpad_up)
          {
            return -1;
          }
          else if (this->xbox360.dpad_down)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_TRIGGER: return (this->xbox360.rt - this->xbox360.lt)/255.0f;

        case XBOX_AXIS_X1: return s16_to_float(this->xbox360.x1);
        case XBOX_AXIS_Y1: return s16_to_float(this->xbox360.y1);
        case XBOX_AXIS_X2: return s16_to_float(this->xbox360.x2);
        case XBOX_AXIS_Y2: return s16_to_float(this->xbox360.y2);
        case XBOX_AXIS_LT: return u8_to_float(this->xbox360.lt);
        case XBOX_AXIS_RT: return u8_to_float(this->xbox360.rt);

        case XBOX_AXIS_A:     return 0;
        case XBOX_AXIS_B:     return 0;
        case XBOX_AXIS_X:     return 0;
        case XBOX_AXIS_Y:     return 0;
        case XBOX_AXIS_BLACK: return 0;
        case XBOX_AXIS_WHITE: return 0;
      }
      break;

    case XBOX_MSG_XBOX:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->xbox.dpad_left)
          {
            return -1;
          }
          else if (this->xbox.dpad_right)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->xbox.dpad_up)
          {
            return -1;
          }
          else if (this->xbox.dpad_down)
          {
            return 1;
          }
          else
          {
            return 0;
          }

        case XBOX_AXIS_TRIGGER: return (this->xbox.rt - this->xbox.lt) / 255.0f;

        case XBOX_AXIS_X1: return s16_to_float(this->xbox.x1);
        case XBOX_AXIS_Y1: return s16_to_float(this->xbox.y1);
        case XBOX_AXIS_X2: return s16_to_float(this->xbox.x2);
        case XBOX_AXIS_Y2: return s16_to_float(this->xbox.y2);
        case XBOX_AXIS_LT: return u8_to_float(this->xbox.lt);
        case XBOX_AXIS_RT: return u8_to_float(this->xbox.rt);

        case XBOX_AXIS_A:     return u8_to_float(this->xbox.a);
        case XBOX_AXIS_B:     return u8_to_float(this->xbox.b);
        case XBOX_AXIS_X:     return u8_to_float(this->xbox.x);
        case XBOX_AXIS_Y:     return u8_to_float(this->xbox.y);
        case XBOX_AXIS_BLACK: return u8_to_float(this->xbox.black);
        case XBOX_AXIS_WHITE: return u8_to_float(this->xbox.white);
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_DPAD_X:
          if (this->ps3usb.dpad_left)
          {
            return -1.0f;
          }
          else if (this->ps3usb.dpad_right)
          {
            return 1.0f;
          }
          else
          {
            return 0.0f;
          }

        case XBOX_AXIS_DPAD_Y:
          if (this->ps3usb.dpad_up)
          {
            return -1.0f;
          }
          else if (this->ps3usb.dpad_down)
          {
            return 1.0f;
          }
          else
          {
            return 0.0f;
          }

        case XBOX_AXIS_TRIGGER: return static_cast<float>(this->ps3usb.r2 - this->ps3usb.l2)/512.0f;

        case XBOX_AXIS_X1: return u8_to_float(this->ps3usb.x1);
        case XBOX_AXIS_Y1: return u8_to_float(this->ps3usb.y1);
        case XBOX_AXIS_X2: return u8_to_float(this->ps3usb.x2);
        case XBOX_AXIS_Y2: return u8_to_float(this->ps3usb.y2);
        case XBOX_AXIS_LT: return u8_to_float(this->ps3usb.a_l2);
        case XBOX_AXIS_RT: return u8_to_float(this->ps3usb.a_r2);

        case XBOX_AXIS_A:     return u8_to_float(this->ps3usb.a_cross);
        case XBOX_AXIS_B:     return u8_to_float(this->ps3usb.a_circle);
        case XBOX_AXIS_X:     return u8_to_float(this->ps3usb.a_square);
        case XBOX_AXIS_Y:     return u8_to_float(this->ps3usb.a_triangle);
        case XBOX_AXIS_BLACK: return u8_to_float(this->ps3usb.a_l1);
        case XBOX_AXIS_WHITE: return u8_to_float(this->ps3usb.a_r1);
      }
  }
  return 0;
}

void
XboxGenericMsg::set_axis_float(XboxAxis axis, float v)
{
  switch(this->type)
  {
    case XBOX_MSG_XBOX360:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_TRIGGER:
          this->xbox360.lt = v < 0 ? static_cast<int>(-v*255) : 0;
          this->xbox360.rt = v > 0 ? static_cast<int>(v*255) : 0;
          break;
        
        case XBOX_AXIS_DPAD_X:
          if (v > 0.5f)
          {
            this->xbox360.dpad_left  = false;
            this->xbox360.dpad_right =  true;
          }
          else if (v < -0.5f)
          {
            this->xbox360.dpad_left   = true;
            this->xbox360.dpad_right = false;
          }
          else
          {
            this->xbox360.dpad_left  = false;
            this->xbox360.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0.5f)
          {
            this->xbox360.dpad_up   = false;
            this->xbox360.dpad_down = true;
          }
          else if (v < -0.5f)
          {
            this->xbox360.dpad_up   = true;
            this->xbox360.dpad_down = false;
          }
          else
          {
            this->xbox360.dpad_down = false;
            this->xbox360.dpad_up  = false;
          }
          break;

        case XBOX_AXIS_X1: this->xbox360.x1 = float_to_s16(v); break;
        case XBOX_AXIS_Y1: this->xbox360.y1 = float_to_s16(v); break;
        case XBOX_AXIS_X2: this->xbox360.x2 = float_to_s16(v); break;
        case XBOX_AXIS_Y2: this->xbox360.y2 = float_to_s16(v); break;
        case XBOX_AXIS_LT: this->xbox360.lt = float_to_u8(v);  break;
        case XBOX_AXIS_RT: this->xbox360.rt = float_to_u8(v);  break;

        case XBOX_AXIS_A:     break;
        case XBOX_AXIS_B:     break;
        case XBOX_AXIS_X:     break;
        case XBOX_AXIS_Y:     break;
        case XBOX_AXIS_BLACK: break;
        case XBOX_AXIS_WHITE: break;
      }
      break;

    case XBOX_MSG_XBOX:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_TRIGGER:
          this->xbox.lt = v < 0 ? static_cast<int>(-v*255) : 0;
          this->xbox.rt = v > 0 ? static_cast<int>(v*255) : 0;
          break;

        case XBOX_AXIS_DPAD_X:
          if (v > 0.5f)
          {
            this->xbox.dpad_left  = false;
            this->xbox.dpad_right =  true;
          }
          else if (v < -0.5f)
          {
            this->xbox.dpad_left   = true;
            this->xbox.dpad_right = false;
          }
          else
          {
            this->xbox.dpad_left  = false;
            this->xbox.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0.5f)
          {
            this->xbox.dpad_up   = false;
            this->xbox.dpad_down = true;
          }
          else if (v < -0.5f)
          {
            this->xbox.dpad_up   = true;
            this->xbox.dpad_down = false;
          }
          else
          {
            this->xbox.dpad_down = false;
            this->xbox.dpad_up   = false;
          }
          break;

        case XBOX_AXIS_X1: this->xbox.x1 = float_to_s16(v); break;
        case XBOX_AXIS_Y1: this->xbox.y1 = float_to_s16(v); break;
        case XBOX_AXIS_X2: this->xbox.x2 = float_to_s16(v); break;
        case XBOX_AXIS_Y2: this->xbox.y2 = float_to_s16(v); break;
        case XBOX_AXIS_LT: this->xbox.lt = float_to_u8(v); break;
        case XBOX_AXIS_RT: this->xbox.rt = float_to_u8(v); break;

        case XBOX_AXIS_A:     this->xbox.a = float_to_u8(v); break;
        case XBOX_AXIS_B:     this->xbox.b = float_to_u8(v); break;
        case XBOX_AXIS_X:     this->xbox.x = float_to_u8(v); break;
        case XBOX_AXIS_Y:     this->xbox.y = float_to_u8(v); break;
        case XBOX_AXIS_BLACK: this->xbox.black = float_to_u8(v); break;
        case XBOX_AXIS_WHITE: this->xbox.white = float_to_u8(v); break;
      }
      break;

    case XBOX_MSG_PS3USB:
      switch(axis)
      {
        case XBOX_AXIS_MAX:
        case XBOX_AXIS_UNKNOWN:
          break;

        case XBOX_AXIS_TRIGGER:
          this->ps3usb.a_l2 = v < 0 ? static_cast<int>(-v*255) : 0;
          this->ps3usb.a_r2 = v > 0 ? static_cast<int>(v*255) : 0;
          break;

        case XBOX_AXIS_DPAD_X:
          if (v > 0.5f)
          {
            this->ps3usb.dpad_left  = false;
            this->ps3usb.dpad_right =  true;
          }
          else if (v < -0.5f)
          {
            this->ps3usb.dpad_left   = true;
            this->ps3usb.dpad_right = false;
          }
          else
          {
            this->ps3usb.dpad_left  = false;
            this->ps3usb.dpad_right = false;
          }
          break;

        case XBOX_AXIS_DPAD_Y:
          if (v > 0.5f)
          {
            this->ps3usb.dpad_up   = false;
            this->ps3usb.dpad_down = true;
          }
          else if (v < -0.5f)
          {
            this->ps3usb.dpad_up   = true;
            this->ps3usb.dpad_down = false;
          }
          else
          {
            this->ps3usb.dpad_down = false;
            this->ps3usb.dpad_up   = false;
          }
          break;

        case XBOX_AXIS_X1: this->ps3usb.x1 = float_to_s16(v); break;
        case XBOX_AXIS_Y1: this->ps3usb.y1 = float_to_s16(v); break;
        case XBOX_AXIS_X2: this->ps3usb.x2 = float_to_s16(v); break;
        case XBOX_AXIS_Y2: this->ps3usb.y2 = float_to_s16(v); break;
        case XBOX_AXIS_LT: this->ps3usb.a_l2 = float_to_u8(v); break;
        case XBOX_AXIS_RT: this->ps3usb.a_r2 = float_to_u8(v); break;

        case XBOX_AXIS_A:     this->ps3usb.a_cross    = float_to_u8(v); break;
        case XBOX_AXIS_B:     this->ps3usb.a_circle   = float_to_u8(v); break;
        case XBOX_AXIS_X:     this->ps3usb.a_square   = float_to_u8(v); break;
        case XBOX_AXIS_Y:     this->ps3usb.a_triangle = float_to_u8(v); break;
        case XBOX_AXIS_BLACK: this->ps3usb.a_l1 = float_to_u8(v); break;
        case XBOX_AXIS_WHITE: this->ps3usb.a_r1 = float_to_u8(v); break;
      }
      break;
  }
}

std::string gamepadtype_to_string(const GamepadType& type)
{
  switch (type)
  {
    case GAMEPAD_XBOX360:
      return "xbox360";

    case GAMEPAD_XBOX360_WIRELESS:
      return "xbox360-wireless";

    case GAMEPAD_XBOX360_PLAY_N_CHARGE: 
      return "xbox360-playncharge";

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

    case GAMEPAD_PLAYSTATION3_USB:
      return "playstation3-usb";

    default:
      assert(!"Unknown gamepad type supplied");
  }
}


std::string gamepadtype_to_macro_string(const GamepadType& type)
{
  switch (type)
  {
    case GAMEPAD_XBOX360: return "GAMEPAD_XBOX360";
    case GAMEPAD_XBOX360_WIRELESS: return "GAMEPAD_XBOX360_WIRELESS";
    case GAMEPAD_XBOX360_PLAY_N_CHARGE: return "GAMEPAD_XBOX360_PLAY_N_CHARGE";
    case GAMEPAD_XBOX: return "GAMEPAD_XBOX";
    case GAMEPAD_XBOX_MAT: return "GAMEPAD_XBOX_MAT";
    case GAMEPAD_XBOX360_GUITAR: return "GAMEPAD_XBOX360_GUITAR";
    case GAMEPAD_FIRESTORM: return "GAMEPAD_FIRESTORM";
    case GAMEPAD_FIRESTORM_VSB: return "GAMEPAD_FIRESTORM_VSB";
    case GAMEPAD_SAITEK_P2500: return "GAMEPAD_SAITEK_P2500";
    case GAMEPAD_PLAYSTATION3_USB: return "GAMEPAD_PLAYSTATION3_USB";
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

    case GAMEPAD_XBOX360_PLAY_N_CHARGE: 
      return out << "Xbox360 Play&Charge";

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

    case GAMEPAD_PLAYSTATION3_USB:
      return out << "Playstation 3 USB";

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

    case XBOX_MSG_PS3USB:
      return out << msg.ps3usb;
        
    default:
      return out << "Error: Unhandled XboxGenericMsg type: " << msg.type;
  }
}

std::ostream& operator<<(std::ostream& out, const Playstation3USBMsg& msg)
{
  out << boost::format("X1:%3d Y1:%3d")
    % int(msg.x1) % int(msg.y1);

  out << boost::format("  X2:%3d Y2:%3d")
    % int(msg.x2) % int(msg.y2);

  out << boost::format("  du:%3d dd:%3d dl:%3d dr:%3d")
    % int(msg.a_dpad_up)
    % int(msg.a_dpad_down)
    % int(msg.a_dpad_left)
    % int(msg.a_dpad_right);

  out << "  select:" << msg.select;
  out << " ps:" << msg.playstation;
  out << " start:" << msg.start;

  out << boost::format("  L3:%d R3:%d") % static_cast<int>(msg.l3) % static_cast<int>(msg.r3);

  out << boost::format("  /\\:%3d O:%3d X:%3d []:%3d  L1:%3d R1:%3d")
    % static_cast<int>(msg.a_triangle)
    % static_cast<int>(msg.a_circle)
    % static_cast<int>(msg.a_cross)
    % static_cast<int>(msg.a_square)
    % static_cast<int>(msg.a_l1)
    % static_cast<int>(msg.a_r1);

  out << boost::format("  L2:%3d R2:%3d")
    % int(msg.a_l2) % int(msg.a_r2);

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

  // out << " Dummy: " << msg.dummy1 << " " << msg.dummy2 << " " << msg.dummy3;

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

  // out << " Dummy: " << msg.dummy;

  return out;
}

XboxButton string2btn(const std::string& str_)
{
  std::string str = to_lower(str_);

  if (str == "start")
    return XBOX_BTN_START;
  else if (str == "guide" || str == "ps")
    return XBOX_BTN_GUIDE;
  else if (str == "back" || str == "select")
    return XBOX_BTN_BACK;

  else if (str == "a" || str == "1" || str == "green" || str == "cross")
    return XBOX_BTN_A;
  else if (str == "b" || str == "2" || str == "red" || str == "circle")
    return XBOX_BTN_B;
  else if (str == "x" || str == "3" || str == "blue" || str == "square")
    return XBOX_BTN_X;
  else if (str == "y" || str == "4" || str == "yellow" || str == "triangle")
    return XBOX_BTN_Y;

  else if (str == "lb" || str == "5" || str == "orange" || str == "white" || str == "l1")
    return XBOX_BTN_LB;
  else if (str == "rb" || str == "6" || str == "black" || str == "r1")
    return XBOX_BTN_RB;

  else if (str == "lt" || str == "7" || str == "l2")
    return XBOX_BTN_LT;
  else if (str == "rt" || str == "8" || str == "r2")
    return XBOX_BTN_RT;

  else if (str == "tl" || str == "l3")
    return XBOX_BTN_THUMB_L;
  else if (str == "tr" || str == "r3")
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
    raise_exception(std::runtime_error, "couldn't convert string \"" + str + "\" to XboxButton");
}

XboxAxis string2axis(const std::string& str_)
{
  std::string str = to_lower(str_);
  if (str == "x1")
    return XBOX_AXIS_X1;
  else if (str == "y1")
    return XBOX_AXIS_Y1;
  
  else if (str == "x2" || str == "whammy")
    return XBOX_AXIS_X2;
  else if (str == "y2" || str == "tilt")
    return XBOX_AXIS_Y2;
  
  else if (str == "lt" || str == "l2")
    return XBOX_AXIS_LT;
  else if (str == "rt" || str == "r2")
    return XBOX_AXIS_RT;

  else if (str == "dpad_x")
    return XBOX_AXIS_DPAD_X;
  else if (str == "dpad_y")
    return XBOX_AXIS_DPAD_Y;

  else if (str == "trigger" || str == "z" || str == "rudder")
    return XBOX_AXIS_TRIGGER;

  else if (str == "btn_a" || str == "cross")
    return XBOX_AXIS_A;

  else if (str == "btn_b" || str == "circle")
    return XBOX_AXIS_B;

  else if (str == "btn_x" || str == "square")
    return XBOX_AXIS_X;

  else if (str == "btn_y" || str == "triangle")
    return XBOX_AXIS_Y;

  else if (str == "white" || str == "lb"|| str == "l1")
    return XBOX_AXIS_WHITE;

  else if (str == "black" || str == "rb" || str == "r1")
    return XBOX_AXIS_BLACK;

  else
    raise_exception(std::runtime_error, "couldn't convert string \"" + str + "\" to XboxAxis");
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

    case XBOX_AXIS_A:     return "BTN_A";
    case XBOX_AXIS_B:     return "BTN_B";
    case XBOX_AXIS_X:     return "BTN_X"; 
    case XBOX_AXIS_Y:     return "BTN_Y"; 
    case XBOX_AXIS_BLACK: return "Black";
    case XBOX_AXIS_WHITE: return "White";
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

int
XboxGenericMsg::get_axis_min(XboxAxis axis)
{
  switch(axis)
  {
    case XBOX_AXIS_X1: return -32768;
    case XBOX_AXIS_Y1: return -32768;
    
    case XBOX_AXIS_X2: return -32768;
    case XBOX_AXIS_Y2: return -32768;

    case XBOX_AXIS_LT: return 0;
    case XBOX_AXIS_RT: return 0;

    case XBOX_AXIS_DPAD_X: return -1;
    case XBOX_AXIS_DPAD_Y: return -1;

    case XBOX_AXIS_TRIGGER: return -255;

    case XBOX_AXIS_A:     return 0;
    case XBOX_AXIS_B:     return 0;
    case XBOX_AXIS_X:     return 0;
    case XBOX_AXIS_Y:     return 0;
    case XBOX_AXIS_BLACK: return 0;
    case XBOX_AXIS_WHITE: return 0;

    default: assert(!"never reached");
  }
}

int
XboxGenericMsg::get_axis_max(XboxAxis axis)
{
  switch(axis)
  {
    case XBOX_AXIS_X1: return 32767;
    case XBOX_AXIS_Y1: return 32767;
    
    case XBOX_AXIS_X2: return 32767;
    case XBOX_AXIS_Y2: return 32767;

    case XBOX_AXIS_LT: return 255;
    case XBOX_AXIS_RT: return 255;

    case XBOX_AXIS_DPAD_X: return 1;
    case XBOX_AXIS_DPAD_Y: return 1;

    case XBOX_AXIS_TRIGGER: return 255;

    case XBOX_AXIS_A:     return 255;
    case XBOX_AXIS_B:     return 255;
    case XBOX_AXIS_X:     return 255;
    case XBOX_AXIS_Y:     return 255;
    case XBOX_AXIS_BLACK: return 255;
    case XBOX_AXIS_WHITE: return 255;

    default: assert(!"never reached");
  }
}

void
XboxGenericMsg::set_axis_min(XboxAxis axis, int value)
{
}

void
XboxGenericMsg::set_axis_max(XboxAxis axis, int value)
{
}
  
/* EOF */
