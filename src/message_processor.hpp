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

#ifndef HEADER_XBOXDRV_DEFAULT_MESSAGE_PROCESSOR_HPP
#define HEADER_XBOXDRV_DEFAULT_MESSAGE_PROCESSOR_HPP

#include "controller_slot_config.hpp"

class Options;
class ControllerOptions;
class ControllerMessageDescriptor;

class MessageProcessor
{
private:
  ControllerSlotConfigPtr m_config;

  ControllerMessageDescriptor m_desc;
  ControllerMessage m_oldmsg; /// last data send to uinput
  XboxButton m_config_toggle_button;

  int m_rumble_gain;
  bool m_rumble_test;
  boost::function<void (uint8_t, uint8_t)> m_rumble_callback;

public:
  MessageProcessor(ControllerSlotConfigPtr config, const Options& opts);
  ~MessageProcessor();

  void init(const ControllerMessageDescriptor& desc);
  void send(const ControllerMessage& msg,
            const ControllerMessageDescriptor& msg_desc,
            int msec_delta);
  void set_rumble(uint8_t lhs, uint8_t rhs);
  void set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback);
  void set_config(int num);
  ControllerSlotConfigPtr get_config() const { return m_config; }

  const ControllerMessageDescriptor& get_message_descriptor() const { return m_desc; }

private:
  MessageProcessor(const MessageProcessor&);
  MessageProcessor& operator=(const MessageProcessor&);
};

#endif

/* EOF */
