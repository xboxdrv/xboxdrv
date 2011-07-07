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

#include "buttonevent/cycle_key_ref_button_event_handler.hpp"

#include <boost/tokenizer.hpp>

#include "buttonevent/cycle_key_button_event_handler.hpp"
#include "raise_exception.hpp"

namespace {

CycleKeyRefButtonEventHandler::Direction 
direction_from_string(const std::string& value)
{
  if (value == "forward")
  {
    return CycleKeyRefButtonEventHandler::kForward;
  }
  else if (value == "backward")
  {
    return CycleKeyRefButtonEventHandler::kBackward;
  }
  else if (value == "none")
  {
    return CycleKeyRefButtonEventHandler::kNone;
  }
  else
  {
    raise_exception(std::runtime_error, "allowed values are 'forward', 'backward' and 'none'");
  }
}

} // namespace

CycleKeyRefButtonEventHandler*
CycleKeyRefButtonEventHandler::from_string(const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (args.size() > 0)
  {
    std::string name = args[0];
    Direction direction = (args.size() > 1) ? direction_from_string(args[1]) : kBackward;
    bool press    = (args.size() > 2) ? boost::lexical_cast<bool>(args[2]) : true;

    CycleKeyButtonEventHandler* cycle = CycleKeyButtonEventHandler::lookup(name);
    if (!cycle)
    {
      raise_exception(std::runtime_error, "need at least one arguments");
    }
    else
    {
      return new CycleKeyRefButtonEventHandler(cycle, direction, press);
    }
  }
  else
  {
    raise_exception(std::runtime_error, "need at least one arguments");
  }
}

CycleKeyRefButtonEventHandler::CycleKeyRefButtonEventHandler(CycleKeyButtonEventHandler* button_handler, 
                                                             Direction direction, 
                                                             bool press) :
  m_button_handler(button_handler),
  m_direction(direction),
  m_send_press(press)
{
  // FIXME: m_button_handler is just a raw pointer without a well
  // defined scope, bad idea, should use a boost::weak_ref<> instead
  // or something like that
}

void
CycleKeyRefButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  // nothing to do, as m_button_handler is doing all the work
}

void
CycleKeyRefButtonEventHandler::send(UInput& uinput, bool value)
{  
  if (value)
  {
    switch(m_direction)
    {
      case kBackward:
        m_button_handler->prev_key();
        break;

      case kForward:
        m_button_handler->next_key();
        break;

      case kNone:
        break;
    }

    if (m_send_press)
    {
      m_button_handler->send_only(uinput, value);
    }
  }
  else
  {
    if (m_send_press)
    {
      m_button_handler->send_only(uinput, value);
    }
  }
}

void
CycleKeyRefButtonEventHandler::update(UInput& uinput, int msec_delta)
{
}

std::string
CycleKeyRefButtonEventHandler::str() const
{
  return "cycle-key-ref";
}

/* EOF */
