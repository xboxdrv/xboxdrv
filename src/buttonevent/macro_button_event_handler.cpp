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

#include "buttonevent/macro_button_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <fstream>
#include <linux/input.h>
#include <vector>

#include "evdev_helper.hpp"
#include "uinput.hpp"

MacroButtonEventHandler*
MacroButtonEventHandler::from_string(const std::string& str)
{
  std::vector<MacroEvent> events;

  std::ifstream in(str.c_str());
  std::string line;
  while(std::getline(in, line))
  {
    MacroEvent ev = macro_event_from_string(line);
    if (ev.type != MacroEvent::kNull)
    {
      events.push_back(ev);
    }
  }
  return new MacroButtonEventHandler(events);
}

MacroButtonEventHandler::MacroEvent
MacroButtonEventHandler::macro_event_from_string(const std::string& str)
{
  MacroEvent event;
  event.type = MacroEvent::kNull;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(" "));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0: 
        if (*t == "send")
        {
          event.type  = MacroEvent::kSendOp;
          event.send.event = UIEvent::invalid();
          event.send.value = 0;
        }
        else if (*t == "wait")
        {
          event.type = MacroEvent::kWaitOp;
          event.wait.msec = 0;
        }
        break;
        
      case 1:
        {
          if (event.type == MacroEvent::kSendOp)
          {
            switch(get_event_type(*t))
            {
              case EV_REL: event.send.event = str2rel_event(*t); break;
              case EV_ABS: event.send.event = str2abs_event(*t); break;
              case EV_KEY: event.send.event = str2key_event(*t); break;
              default: throw std::runtime_error("unknown event type");
            }
          }
          else if (event.type == MacroEvent::kWaitOp)
          {
            event.wait.msec = boost::lexical_cast<int>(*t);
          }
        }
        break;

      case 2:
        {
          if (event.type == MacroEvent::kSendOp)
          {
            event.send.value = boost::lexical_cast<int>(*t);
          }
          else
          {
            throw std::runtime_error("to many arguments for 'wait'");
          }
        }
        break;
    }
  }

  return event;
}

MacroButtonEventHandler::MacroButtonEventHandler(const std::vector<MacroEvent>& events) :
  m_events(events),
  m_send_in_progress(false),
  m_countdown(0),
  m_event_counter()
{
}

void
MacroButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  for(std::vector<MacroEvent>::iterator i = m_events.begin(); i != m_events.end(); ++i)
  {
    switch(i->type)
    {
      case MacroEvent::kSendOp:
        switch(i->send.event.type)
        {
          case EV_REL:
            i->send.event.resolve_device_id(slot, extra_devices),
            uinput.add_rel(i->send.event.get_device_id(), i->send.event.code);
            break;

          case EV_KEY:
            i->send.event.resolve_device_id(slot, extra_devices),
            uinput.add_key(i->send.event.get_device_id(), i->send.event.code);
            break;

          case EV_ABS:
            //i->send.event.resolve_device_id(slot, extra_devices);
            // uinput.add_abs(i->send.event.get_device_id(), i->send.event.code);
            break;

          default:
            assert(!"not implemented");
            break;
        }
        break;

      default:
        // nothing to do
        break;
    }
  }
}

void
MacroButtonEventHandler::send(UInput& uinput, bool value)
{
  if (value && !m_send_in_progress)
  {
    m_send_in_progress = true;
    m_event_counter = 0;
    m_countdown = 0;
  }
}

void
MacroButtonEventHandler::update(UInput& uinput, int msec_delta)
{
  if (m_send_in_progress)
  {
    m_countdown -= msec_delta;
    if (m_countdown <= 0)
    {
      while(true)
      {
        switch(m_events[m_event_counter].type)
        {
          case MacroEvent::kSendOp:
            uinput.send(m_events[m_event_counter].send.event.get_device_id(),
                        m_events[m_event_counter].send.event.type,
                        m_events[m_event_counter].send.event.code,
                        m_events[m_event_counter].send.value);
            break;

          case MacroEvent::kWaitOp:
            m_countdown = m_events[m_event_counter].wait.msec;
            if (m_countdown > 0)
            {
              m_event_counter += 1;
              return;
            }
            break;

          default:
            assert(!"never reached");
            break;
        }

        m_event_counter += 1;

        if (m_event_counter == m_events.size())
        {
          m_send_in_progress = false;
          m_event_counter = 0;
          m_countdown = 0;
          return;
        }
      }
    }
  }
}

std::string
MacroButtonEventHandler::str() const
{
  return "macro";
}

/* EOF */
