/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#ifndef HEADER_XBOX360_DRIVER_HPP
#define HEADER_XBOX360_DRIVER_HPP

#include <vector>
#include "xboxdrv.hpp"
#include "control.hpp"

/** */
class Xbox360Driver : public Control
{
public:
  enum { 
    XBOX360_DPAD_UP, 
    XBOX360_DPAD_DOWN, 
    XBOX360_DPAD_LEFT, 
    XBOX360_DPAD_RIGHT, 

    XBOX360_BTN_A, 
    XBOX360_BTN_B, 
    XBOX360_BTN_X,
    XBOX360_BTN_Y, 
    XBOX360_BTN_LB, 
    XBOX360_BTN_RB, 
    XBOX360_BTN_THUMB_L, 
    XBOX360_BTN_THUMB_R,

    XBOX360_BTN_START, 
    XBOX360_BTN_BACK, 
    XBOX360_BTN_GUIDE, 

    XBOX360_BTN_LENGTH, 
  };

  enum {
    XBOX360_AXIS_X1,
    XBOX360_AXIS_Y1,

    XBOX360_AXIS_X2,
    XBOX360_AXIS_Y2,

    XBOX360_AXIS_LT,
    XBOX360_AXIS_RT,

    XBOX360_ABS_MAX
  };

  enum {
    BTN_PORT_IN_LED,
    BTN_PORT_IN_MAX
  };

  enum {
    ABS_PORT_IN_RUMBLE_L,
    ABS_PORT_IN_RUMBLE_R,
    ABS_PORT_IN_MAX,
  };

private:
  struct usb_device*     dev;
  struct usb_dev_handle* handle;
  
  uint8_t rumble_l;
  uint8_t rumble_r;

public:
  Xbox360Driver(int idx);
  Xbox360Driver(const std::string& busid, const std::string& devid);
  ~Xbox360Driver();

  void set_led(uint8_t led_status);
  void set_rumble(uint8_t big, uint8_t small);
  
  void on_led_btn(BtnPortOut* btn);

  void on_rumble_left_abs(AbsPortOut* abs);
  void on_rumble_right_abs(AbsPortOut* abs);

  void update(float delta);

private:
  void init();
  void open_dev();
  void close_dev();
  void process_msg(const Xbox360Msg& msg);

  Xbox360Driver (const Xbox360Driver&);
  Xbox360Driver& operator= (const Xbox360Driver&);
};

#endif

/* EOF */
