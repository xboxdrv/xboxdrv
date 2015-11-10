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

#include "buttonevent/macro_button_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <fstream>
#include <linux/input.h>
#include <vector>

#include "evdev_helper.hpp"
#include "helper.hpp"
#include "log.hpp"
#include "raise_exception.hpp"
#include "uinput.hpp"

MacroButtonEventHandler*
MacroButtonEventHandler::from_string(UInput& uinput, int slot, bool extra_devices,
                                     const std::string& str)
{
  std::vector<MacroEvent> events;
  for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
  {
    { // push
      MacroEvent event;
      event.type  = MacroEvent::kSendOp;
      event.send.event = UIEvent::from_char(*i);
      event.send.emitter = 0;
      event.send.value = 1;
      events.push_back(event);
    }
    { // wait
      MacroEvent event;
      event.type  = MacroEvent::kWaitOp;
      event.wait.msec = 20;
      events.push_back(event);
    }
    { // release
      MacroEvent event;
      event.type  = MacroEvent::kSendOp;
      event.send.event = UIEvent::from_char(*i);
      event.send.emitter = 0;
      event.send.value = 0;
      events.push_back(event);
    }
  }

  return new MacroButtonEventHandler(uinput, slot, extra_devices, events);
}

MacroButtonEventHandler*
MacroButtonEventHandler::from_file(UInput& uinput, int slot, bool extra_devices,
                                   const std::string& filename)
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
    return new MacroButtonEventHandler(uinput, slot, extra_devices, events);
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
        event.init.emitter = 0;
        event.init.minimum = str2int(args[2]);
        event.init.maximum = str2int(args[3]);
        event.init.fuzz = 0;
        event.init.flat = 0;
        if (args.size() > 4) event.init.fuzz = str2int(args[4]);
        if (args.size() > 5) event.init.flat = str2int(args[5]);

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
        event.send.emitter = 0;
        event.send.value = str2int(args[2]);
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
        event.wait.msec = str2int(args[1]);
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

MacroButtonEventHandler::MacroButtonEventHandler(UInput& uinput, int slot, bool extra_devices,
                                                 const std::vector<MacroEvent>& events) :
  m_events(events),
  m_send_in_progress(false),
  m_countdown(0),
  m_event_counter(),
  m_emitter()
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
            i->init.emitter = new UIEventEmitterPtr(
              uinput.add_abs(i->init.event.get_device_id(), i->init.event.code,
                             i->init.minimum, i->init.maximum,
                             i->init.fuzz, i->init.flat));
            break;

          default:
            assert(!"not implemented");
            break;
        }
        break;

      case MacroEvent::kSendOp:
        switch(i->send.event.type)
        {
          case EV_REL:
            i->send.event.resolve_device_id(slot, extra_devices);
            i->send.emitter = new UIEventEmitterPtr(get_emitter(uinput, i->send.event));
            break;

          case EV_KEY:
            i->send.event.resolve_device_id(slot, extra_devices);
            i->send.emitter = new UIEventEmitterPtr(get_emitter(uinput, i->send.event));
            break;

          case EV_ABS:
            i->send.event.resolve_device_id(slot, extra_devices);
            // BROKEN: need to get the UIEventEmitterPtr inited earlier
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

MacroButtonEventHandler::~MacroButtonEventHandler()
{
  for(std::vector<MacroEvent>::iterator i = m_events.begin(); i != m_events.end(); ++i)
  {
    switch(i->type)
    {
      case MacroEvent::kInitOp:
        delete i->init.emitter;
        break;

      case MacroEvent::kSendOp:
        delete i->send.emitter;
        break;

      default:
        // do nothing
        break;
    }
  }
}

UIEventEmitterPtr
MacroButtonEventHandler::get_emitter(UInput& uinput, const UIEvent& ev)
{
  Emitter::iterator it = m_emitter.find(ev);
  if (it != m_emitter.end())
  {
    return it->second;
  }
  else
  {
    UIEventEmitterPtr emitter = uinput.add(ev);
    m_emitter[ev] = emitter;
    return emitter;
  }
}

void
MacroButtonEventHandler::send(bool value)
{
  if (value && !m_send_in_progress)
  {
    m_send_in_progress = true;
    m_event_counter = 0;
    m_countdown = 0;
  }
}

void
MacroButtonEventHandler::update(int msec_delta)
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
            (*m_events[m_event_counter].send.emitter)->send(m_events[m_event_counter].send.value);
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
