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

#include <vector>

#include "controller_config_set.hpp"
#include "message_processor.hpp"

class uInput;
class Options;

class DefaultMessageProcessor : public MessageProcessor
{
private:
  uInput& m_uinput;
  ControllerConfigSet m_config;

  XboxGenericMsg m_oldmsg; /// last data send to uinput

public:
  DefaultMessageProcessor(uInput& uinput, const Options& opts);
  ~DefaultMessageProcessor();

  void send(XboxGenericMsg& msg, int msec_delta);

private:
  void create_modifier(const Options& opts, std::vector<ModifierPtr>* modifier);

private:
  DefaultMessageProcessor(const DefaultMessageProcessor&);
  DefaultMessageProcessor& operator=(const DefaultMessageProcessor&);
};

#endif

/* EOF */
