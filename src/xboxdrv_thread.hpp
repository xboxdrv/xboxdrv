/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_XBOXDRV_THREAD_HPP
#define HEADER_XBOXDRV_XBOXDRV_THREAD_HPP

#include <memory>
#include <boost/thread.hpp>

#include "xboxmsg.hpp"

class Options;
class uInput;
class XboxGenericController;

class XboxdrvThread // FIXME: find a better name, XboxdrvControllerLoop?!
{
private:
  std::auto_ptr<boost::thread> m_thread;

public:
  XboxdrvThread();

  void controller_loop(GamepadType type, uInput* uinput, XboxGenericController* controller, 
                       const Options& opts);

  void launch_thread(GamepadType type, uInput* uinput, XboxGenericController* controller, 
                       const Options& opts);
  
private:
  XboxdrvThread(const XboxdrvThread&);
  XboxdrvThread& operator=(const XboxdrvThread&);
};

#endif

/* EOF */
