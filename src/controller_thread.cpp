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
#include <functional>
#include <glib.h>

#include <logmich/log.hpp>

#include "controller.hpp"
#include "controller_slot_config.hpp"
#include "message_processor.hpp"

namespace xboxdrv {

using namespace std::placeholders;

extern bool global_exit_xboxdrv;

ControllerThread::ControllerThread(ControllerPtr controller,
                                   ControllerSlotConfigPtr config,
                                   const Options& opts) :
  m_controller(controller),
  m_processor(new MessageProcessor(config, m_controller->get_message_descriptor(), opts)),
  m_oldrealmsg(),
  m_timeout(opts.timeout),
  m_print_messages(!opts.silent),
  m_timeout_id(),
  m_timer(g_timer_new())
{
  m_timeout_id = g_timeout_add(m_timeout, &ControllerThread::on_timeout_wrap, this);
  m_controller->set_message_cb(std::bind(&ControllerThread::on_message, this, _1));
  if (m_processor.get())
  {
    m_processor->set_ff_callback(std::bind(&Controller::set_rumble, m_controller.get(), _1, _2));
  }
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
    int msec_delta = static_cast<int>(g_timer_elapsed(m_timer, nullptr) * 1000.0);
    g_timer_reset(m_timer);

    m_processor->send(m_oldrealmsg, m_controller->get_message_descriptor(), msec_delta);
  }

  return true; // do not remove the callback
}

void
ControllerThread::on_message(const ControllerMessage& msg)
{
  if (m_print_messages)
  {
    std::cout << "msg: ";
    format_generic(std::cout, msg, m_controller->get_message_descriptor()) << std::endl;
  }

  m_oldrealmsg = msg;

  int msec_delta = static_cast<int>(g_timer_elapsed(m_timer, nullptr) * 1000.0);
  g_timer_reset(m_timer);

  if (m_processor.get())
  {
    m_processor->send(msg, m_controller->get_message_descriptor(), msec_delta);
  }
}

} // namespace xboxdrv

/* EOF */
