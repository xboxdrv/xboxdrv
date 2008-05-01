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

#ifndef HEADER_XBOX360_CONTROLLER_HPP
#define HEADER_XBOX360_CONTROLLER_HPP

#include <usb.h>

struct XPadDevice;

/** */
class Xbox360Controller
{
private:
  struct usb_device* dev;
  XPadDevice*        dev_type;
  struct usb_dev_handle* handle;
  
public:
  Xbox360Controller(struct usb_device* dev,
                    XPadDevice*        dev_type);

  void set_rumble(uint8_t left, uint8_t right);
  void set_led(uint8_t status);
  void read(Xbox360Msg& msg);

private:
  Xbox360Controller (const Xbox360Controller&);
  Xbox360Controller& operator= (const Xbox360Controller&);
};

#endif

/* EOF */
