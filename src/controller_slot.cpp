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

#include "controller_slot.hpp"

#include <boost/format.hpp>

#include "uinput_message_processor.hpp"
#include "dummy_message_processor.hpp"

ControllerSlot::ControllerSlot(int id_,
                               ControllerSlotConfigPtr config_,
                               std::vector<ControllerMatchRulePtr> rules_,
                               int led_status_,
                               const Options& opts,
                               UInput* uinput) :
  m_id(id_),
  m_config(config_),
  m_rules(rules_),
  m_led_status(led_status_),
  m_thread(),
  m_opts(opts),
  m_uinput(uinput)
{}

void
ControllerSlot::connect(ControllerPtr controller)
{
  assert(!m_thread);

  m_thread.reset(new ControllerThread(controller, m_opts));

  if (m_uinput)
  {
    m_thread->set_message_proc(std::auto_ptr<MessageProcessor>(new UInputMessageProcessor(*m_uinput, m_config, m_opts)));
  }
  else
  {
    m_thread->set_message_proc(std::auto_ptr<MessageProcessor>(new DummyMessageProcessor()));
  }

  m_thread->start();
}

ControllerPtr
ControllerSlot::disconnect()
{
  assert(m_thread);

  ControllerPtr controller = m_thread->get_controller();
  m_thread->stop();
  m_thread.reset();

  return controller;
}

bool
ControllerSlot::is_connected() const
{
  return m_thread;
}

/* EOF */
