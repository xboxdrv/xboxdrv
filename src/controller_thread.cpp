/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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
#include <glib.h>

#include "helper.hpp"
#include "log.hpp"
#include "controller.hpp"
#include "message_processor.hpp"

extern bool global_exit_xboxdrv;

ControllerThread::ControllerThread(ControllerPtr controller,
                                   std::auto_ptr<MessageProcessor> processor,
                                   const Options& opts) :
  m_controller(controller),
  m_processor(processor),
  m_oldrealmsg(),
  m_timeout(opts.timeout),
  m_print_messages(!opts.silent),
  m_timeout_id(),
  m_timer(g_timer_new())
{
  memset(&m_oldrealmsg, 0, sizeof(m_oldrealmsg));
  m_timeout_id = g_timeout_add(m_timeout, &ControllerThread::on_timeout_wrap, this);
  m_controller->set_message_cb(boost::bind(&ControllerThread::on_message, this, _1));
  m_processor->set_ff_callback(boost::bind(&Controller::set_rumble, m_controller.get(), _1, _2));
}

ControllerThread::~ControllerThread()
{
  g_source_remove(m_timeout_id);
  g_timer_destroy(m_timer);
}

bool
ControllerThread::on_timeout()
{
  if (m_processor.get())
  {
    int msec_delta = static_cast<int>(g_timer_elapsed(m_timer, NULL) * 1000.0f);
    g_timer_reset(m_timer);

    m_processor->send(m_oldrealmsg, msec_delta);
  }

  return true; // do not remove the callback
}

void
ControllerThread::on_message(const XboxGenericMsg& msg)
{
  if (m_print_messages)
  {
    std::cout << msg << std::endl;
  }

  m_oldrealmsg = msg;

  int msec_delta = static_cast<int>(g_timer_elapsed(m_timer, NULL) * 1000.0f);
  g_timer_reset(m_timer);

  if (m_processor.get())
  {
    m_processor->send(msg, msec_delta);
  }
}

/* EOF */
