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

#ifndef HEADER_XBOXDRV_XBOXDRV_THREAD_HPP
#define HEADER_XBOXDRV_XBOXDRV_THREAD_HPP

#include <boost/shared_ptr.hpp>
#include <glib.h>

#include "controller_message.hpp"
#include "controller_ptr.hpp"
#include "controller_slot_ptr.hpp"

class Options;
class MessageProcessor;
class ControllerThread;

class ControllerSlotConfig;

typedef boost::shared_ptr<ControllerSlotConfig> ControllerSlotConfigPtr;
typedef boost::shared_ptr<ControllerThread> ControllerThreadPtr;

/** ControllerThread handles a single Controller, reads it messages
    and passes it to the MessageProcessor */
class ControllerThread // FIXME: find a better name,ControllerLoop?!
{
private:
  ControllerPtr m_controller;
  std::unique_ptr<MessageProcessor> m_processor;

  ControllerMessage m_oldrealmsg; /// last data read from the device

  int  m_timeout;
  bool m_print_messages;
  guint m_timeout_id;
  GTimer* m_timer;

public:
  ControllerThread(ControllerPtr controller, ControllerSlotConfigPtr config, const Options& opts);
  ~ControllerThread();

  MessageProcessor* get_message_proc() const { return m_processor.get(); }
  ControllerPtr get_controller() const { return m_controller; }

private:
  void on_message(const ControllerMessage& msg);

  bool on_timeout();
  static gboolean on_timeout_wrap(gpointer data) {
    return static_cast<ControllerThread*>(data)->on_timeout();
  }

private:
  ControllerThread(const ControllerThread&);
  ControllerThread& operator=(const ControllerThread&);
};

#endif

/* EOF */

