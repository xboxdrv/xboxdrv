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
#include "modifier.hpp"
#include "controller_config_set.hpp"

class Options;
class XboxGenericController;
class MessageProcessor;

class XboxdrvThread // FIXME: find a better name, XboxdrvControllerLoop?!
{
private:
  std::auto_ptr<boost::thread> m_thread;
  std::auto_ptr<MessageProcessor> m_processor;
  std::auto_ptr<XboxGenericController> m_controller;
  bool m_loop;

  XboxGenericMsg m_oldrealmsg; /// last data read from the device

  std::vector<std::string> m_child_exec;
  pid_t m_pid;

  int m_timeout;

public:
  XboxdrvThread(std::auto_ptr<MessageProcessor> processor,
                std::auto_ptr<XboxGenericController> controller,
                const Options& opts);
  ~XboxdrvThread();

  // main loop, can be started in a separate thread with
  // start_thread() or used in its own in the main thread
  void controller_loop(const Options& opts);

  // thread control functions
  void start_thread(const Options& opts);
  void stop_thread();
  bool try_join_thread();

private:
  void launch_child_process();
  void watch_chid_process();

private:
  XboxdrvThread(const XboxdrvThread&);
  XboxdrvThread& operator=(const XboxdrvThread&);
};

#endif

/* EOF */
