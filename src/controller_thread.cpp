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
#include <boost/bind.hpp>

#include "helper.hpp"
#include "log.hpp"
#include "controller.hpp"
#include "message_processor.hpp"

extern bool global_exit_xboxdrv;

ControllerThread::ControllerThread(ControllerPtr controller, const Options& opts) :
  m_processor(),
  m_controller(controller),
  m_oldrealmsg(),
  m_child_exec(opts.exec),
  m_pid(-1),
  m_timeout(opts.timeout),
  m_timeout_id()
{
  memset(&m_oldrealmsg, 0, sizeof(m_oldrealmsg));
}

ControllerThread::~ControllerThread()
{
}

bool
ControllerThread::on_timeout()
{
  //log_debug("timeout time: ");
  // calculate msec delta

  return true; // do not remove the callback
}

void
ControllerThread::on_message(const XboxGenericMsg& msg)
{
  log_trace();
  // calculate msec delta here
  if (m_processor.get())
  {
    m_processor->send(msg);
  }
}

void
ControllerThread::start()
{
  m_controller->start();
  m_timeout_id = g_timeout_add(m_timeout, &ControllerThread::on_timeout_wrap, this);
}

void
ControllerThread::stop()
{
  m_controller->stop();
  g_source_remove(m_timeout_id);
}

void
ControllerThread::set_message_proc(std::auto_ptr<MessageProcessor> processor)
{
  m_processor = processor;
  m_controller->set_message_cb(boost::bind(&ControllerThread::on_message, this, _1));

  // connect the processor to the controller to allow rumble
  if (m_processor.get())
  {
    m_processor->set_ff_callback(boost::bind(&Controller::set_rumble, m_controller.get(), _1, _2));
  }
}

/* EOF */
