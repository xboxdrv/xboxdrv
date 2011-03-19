#if 0
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

#include "controller_thread.hpp"

#include <iostream>
#include <sys/wait.h>

#include "helper.hpp"
#include "log.hpp"
#include "controller.hpp"
#include "message_processor.hpp"

extern bool global_exit_xboxdrv;

// FIXME: isolate problametic code to a separate file, instead of pragma
#pragma GCC diagnostic ignored "-Wold-style-cast"

ControllerThread::ControllerThread(ControllerPtr controller,
                                   const Options& opts) :
  m_processor(),
  m_controller(controller),
  m_loop(true),
  m_oldrealmsg(),
  m_child_exec(opts.exec),
  m_pid(-1),
  m_timeout(opts.timeout),
  m_compatible_slots()
{
  memset(&m_oldrealmsg, 0, sizeof(m_oldrealmsg));
}

ControllerThread::~ControllerThread()
{
}

void
ControllerThread::set_message_proc(std::auto_ptr<MessageProcessor> processor)
{
  m_processor = processor;

  // connect the processor to the controller to allow rumble
  if (m_processor.get())
  {
    m_processor->set_ff_callback(boost::bind(&Controller::set_rumble, m_controller.get(), _1, _2));
  }
}

bool
ControllerThread::is_active() const
{
  return m_controller->is_active();
}

std::string
ControllerThread::get_usbpath() const
{
  return m_controller->get_usbpath();
}
   
std::string 
ControllerThread::get_usbid() const
{
  return m_controller->get_usbid();
}

std::string
ControllerThread::get_name() const
{
  return m_controller->get_name();
}

std::vector<ControllerSlotWeakPtr> 
ControllerThread::get_compatible_slots() const
{
  return m_compatible_slots;
}

void
ControllerThread::set_compatible_slots(const std::vector<ControllerSlotPtr>& slots)
{
  m_compatible_slots.clear();
  for(std::vector<ControllerSlotPtr>::const_iterator i = slots.begin(); i != slots.end(); ++i)
  {
    m_compatible_slots.push_back(*i);
  }
}

/* EOF */
#endif
