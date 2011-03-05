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

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "controller_slot_config.hpp"
#include "controller_slot_ptr.hpp"
#include "controller_ptr.hpp"

class Options;
class MessageProcessor;
class ControllerThread;

typedef boost::shared_ptr<ControllerThread> ControllerThreadPtr;

/** ControllerThread handles a single Controller (optionally in a
    separate thread), reads it messages and passes it to the
    MessageProcessor */
class ControllerThread // FIXME: find a better name, XboxdrvControllerLoop?!
{
private:
  std::auto_ptr<boost::thread> m_thread;
  std::auto_ptr<MessageProcessor> m_processor;
  ControllerPtr m_controller;
  bool m_loop;

  XboxGenericMsg m_oldrealmsg; /// last data read from the device

  std::vector<std::string> m_child_exec;
  pid_t m_pid;

  int m_timeout;

  std::vector<ControllerSlotWeakPtr> m_compatible_slots;

public:
  ControllerThread(ControllerPtr controller,
                   const Options& opts);
  ~ControllerThread();

  // main loop, can be started in a separate thread with
  // start_thread() or used in its own in the main thread
  void controller_loop(const Options& opts);

  // thread control functions
  void start_thread(const Options& opts);
  void stop_thread();
  bool try_join_thread();

  bool is_active() const;

  std::string get_usbpath() const;
  std::string get_usbid() const;
  std::string get_name() const;

  std::vector<ControllerSlotWeakPtr> get_compatible_slots() const;
  void set_compatible_slots(const std::vector<ControllerSlotPtr>& slots);

  void set_message_proc(std::auto_ptr<MessageProcessor> processor);
  MessageProcessor* get_message_proc() const { return m_processor.get(); }

  ControllerPtr get_controller() const { return m_controller; }

private:
  void launch_child_process();
  void watch_chid_process();

private:
  ControllerThread(const ControllerThread&);
  ControllerThread& operator=(const ControllerThread&);
};

#endif

/* EOF */
