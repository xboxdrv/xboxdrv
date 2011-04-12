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
#include "log.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

MacroButtonEventHandler*
MacroButtonEventHandler::from_string(const std::string& filename)
{
  std::vector<MacroEvent> events;

  std::ifstream in(filename.c_str());
  if (!in)
  {
    raise_exception(std::runtime_error, "couldn't open: " << filename);
  }
  else
  {
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
}

MacroButtonEventHandler::MacroEvent
MacroButtonEventHandler::macro_event_from_string(const std::string& str)
{
  boost::tokenizer<boost::char_separator<char> > tokens(str, boost::char_separator<char>(" "));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (args.size() >= 1)
  {
    if (!args[0].empty() && args[0][0] == '#')
    {
      // ignore '#' comments      
      MacroEvent event;
      event.type = MacroEvent::kNull;
      return event;
    }
    else if (args[0] == "init")
    {
      // FIXME: generalize this for EV_KEY and EV_REL
      if (args.size() < 4)
      {
        raise_exception(std::runtime_error, "'init' requires at least three arguments: " << str);
      }
      else
      {
        MacroEvent event;
        event.type = MacroEvent::kInitOp;
        event.init.event = UIEvent::from_string(args[1]);
        event.init.minimum = boost::lexical_cast<int>(args[2]);
        event.init.maximum = boost::lexical_cast<int>(args[3]);
        event.init.fuzz = 0;
        event.init.flat = 0;
        if (args.size() > 4) event.init.fuzz = boost::lexical_cast<int>(args[4]);
        if (args.size() > 5) event.init.flat = boost::lexical_cast<int>(args[5]);

        return event;
      }
    }
    else if (args[0] == "send")
    {
      if (args.size() != 3)
      {
        raise_exception(std::runtime_error, "'send' requires two arguments: " << str);
      }
      else
      {
        MacroEvent event;
        event.type  = MacroEvent::kSendOp;
        event.send.event = UIEvent::from_string(args[1]);
        event.send.value = boost::lexical_cast<int>(args[2]);
        return event;
      }
    }
    else if (args[0] == "wait")
    {
      if (args.size() != 2)
      {
        raise_exception(std::runtime_error, "'wait' requires one arguments: " << str);
      }
      else
      {
        MacroEvent event;
        event.type = MacroEvent::kWaitOp;
        event.wait.msec = boost::lexical_cast<int>(args[1]);
        return event;
      }
    }
    else
    {
      raise_exception(std::runtime_error, "unknown macro command: " << str);
    }
  }
  else
  {
    // no args, aka an empty line, just ignore it
    MacroEvent event;
    event.type = MacroEvent::kNull;
    return event;
  }
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
      case MacroEvent::kInitOp:
        switch(i->init.event.type)
        {
          case EV_REL:
            assert(!"not implemented");
            break;

          case EV_KEY:
            assert(!"not implemented");
            break;

          case EV_ABS:
            i->init.event.resolve_device_id(slot, extra_devices);
            uinput.add_abs(i->init.event.get_device_id(), i->init.event.code,
                           i->init.minimum, i->init.maximum, 
                           i->init.fuzz, i->init.flat);
            break;

          default:
            assert(!"not implemented");
        }
        break;

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
            i->send.event.resolve_device_id(slot, extra_devices);
            // not doing a add_abs() here, its the users job to use a
            // init command for that
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
          case MacroEvent::kInitOp:
            break;

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
              if (m_event_counter >= m_events.size())
              {
                m_send_in_progress = false;
                m_event_counter = 0;
                m_countdown = 0;
                return;
              }
              return;
            }
            break;

          default:
            assert(!"never reached");
            break;
        }

        m_event_counter += 1;

        if (m_event_counter >= m_events.size())
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
