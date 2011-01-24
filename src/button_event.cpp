/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#include "button_event.hpp"

#include <boost/tokenizer.hpp>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "uinput.hpp"

ButtonEventPtr
ButtonEvent::invalid()
{
  return ButtonEventPtr();
}

ButtonEventPtr
ButtonEvent::create(ButtonEventHandler* handler)
{
  return ButtonEventPtr(new ButtonEvent(handler));
}

ButtonEventPtr
ButtonEvent::create_abs(int code)
{
  return ButtonEvent::create(new AbsButtonEventHandler(code));
}

ButtonEventPtr
ButtonEvent::create_key(int code)
{
  return ButtonEvent::create(new KeyButtonEventHandler(code));
}

ButtonEventPtr
ButtonEvent::create_key()
{
  return ButtonEvent::create(new KeyButtonEventHandler);
}

ButtonEventPtr
ButtonEvent::create_rel(int code)
{
  return ButtonEvent::create(new RelButtonEventHandler(UIEvent::create(DEVICEID_AUTO, EV_REL, code)));
}

ButtonEventPtr
ButtonEvent::from_string(const std::string& str)
{
  std::string::size_type p = str.find(':');
  const std::string& token = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (token == "abs")
  {
    return ButtonEvent::create(AbsButtonEventHandler::from_string(rest));
  }
  else if (token == "rel")
  {
    return ButtonEvent::create(RelButtonEventHandler::from_string(rest));
  }
  else if (token == "key")
  {
    return ButtonEvent::create(KeyButtonEventHandler::from_string(rest));
  }
  else if (token == "exec")
  {
    return ButtonEvent::create(ExecButtonEventHandler::from_string(rest));
  }
  else if (token == "macro")
  {
    return ButtonEvent::create(MacroButtonEventHandler::from_string(rest));
  }
  else
  {
    // try to guess the type of event on the type of the first event code
    switch(get_event_type(token))
    {
      case EV_KEY: return ButtonEvent::create(KeyButtonEventHandler::from_string(str));
      case EV_REL: return ButtonEvent::create(RelButtonEventHandler::from_string(str));
      case EV_ABS: return ButtonEvent::create(AbsButtonEventHandler::from_string(str));
      case     -1: return ButtonEvent::invalid(); // void
      default: assert(!"unknown type");
    }
  }
}

ButtonEvent::ButtonEvent(ButtonEventHandler* handler) :
  m_last_send_state(false),
  m_last_raw_state(false),
  m_handler(handler),
  m_filters()
{
}

void
ButtonEvent::add_filters(const std::vector<ButtonFilterPtr>& filters)
{
  std::copy(filters.begin(), filters.end(), std::back_inserter(m_filters));
}

void
ButtonEvent::add_filter(ButtonFilterPtr filter)
{
  m_filters.push_back(filter);
}

void
ButtonEvent::init(uInput& uinput, int slot, bool extra_devices) const
{
  return m_handler->init(uinput, slot, extra_devices);
}

void
ButtonEvent::send(uInput& uinput, bool raw_state)
{
  m_last_raw_state = raw_state;
  bool filtered_state = raw_state;

  // filter values
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    filtered_state = (*i)->filter(filtered_state);
  }

  if (m_last_send_state == filtered_state)
  {
    // button state has not changed, so do not send events
  }
  else
  {
    m_last_send_state = filtered_state;
    m_handler->send(uinput, m_last_send_state);
  }
}

void
ButtonEvent::update(uInput& uinput, int msec_delta)
{
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    (*i)->update(msec_delta);
  }

  m_handler->update(uinput, msec_delta);
  
  send(uinput, m_last_raw_state);
}

std::string
ButtonEvent::str() const
{
  return m_handler->str();
}

KeyButtonEventHandler*
KeyButtonEventHandler::from_string(const std::string& str)
{
  //std::cout << " KeyButtonEventHandler::from_string: " << str << std::endl;

  std::auto_ptr<KeyButtonEventHandler> ev;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0: 
        {
          ev.reset(new KeyButtonEventHandler());

          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end() && k < MAX_MODIFIER; ++m, ++k)
          {
            ev->m_codes[k] = str2key_event(*m);
          }
        }
        break;

      case 1:
        {
          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end() && k < MAX_MODIFIER; ++m, ++k)
          {
            ev->m_secondary_codes[k] = str2key_event(*m);
          }

          ev->m_hold_threshold = 250;
        }
        break;
        
      case 2:
        {
          ev->m_hold_threshold = boost::lexical_cast<int>(*i);
        }
        break;

      default:
        {
          std::ostringstream out;
          out << "to many arguments in '" << str << "'";
          throw std::runtime_error(out.str());
        }
        break;
    }
  }

  return ev.release();
}

KeyButtonEventHandler::KeyButtonEventHandler() :
  m_state(false),
  m_codes(),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
}

KeyButtonEventHandler::KeyButtonEventHandler(int code) :
  m_state(false),
  m_codes(),
  m_secondary_codes(),
  m_hold_threshold(0),
  m_hold_counter(0)
{
  std::fill_n(m_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  std::fill_n(m_secondary_codes, MAX_MODIFIER + 1, UIEvent::invalid());
  m_codes[0] = UIEvent::create(DEVICEID_AUTO, EV_KEY, code);
}

void
KeyButtonEventHandler::init(uInput& uinput, int slot, bool extra_devices) const
{
  for(int i = 0; m_codes[i].is_valid(); ++i)
  {
    uinput.add_key(m_codes[i].device_id, m_codes[i].code);
  }

  if (m_hold_threshold)
  {
    for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
    {
      uinput.add_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code);
    }
  }
}

void
KeyButtonEventHandler::send(uInput& uinput, bool value)
{
  if (m_state != value)
  {
    m_state = value;

    if (m_hold_threshold == 0)
    {
      // FIXME: should handle key releases in reverse order
      for(int i = 0; m_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_codes[i].device_id, m_codes[i].code, m_state);
      }
    }
    else
    {
      if (m_hold_counter < m_hold_threshold)
      {
        if (m_state)
        {
          // we are only sending events after release or when
          // hold_threshold is passed
        }
        else
        {
          // send both a press and release event after another, aka a "click"
          for(int i = 0; m_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_codes[i].device_id, m_codes[i].code, true);
          }
          // FIXME: should do this in reverse order
          for(int i = 0; m_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_codes[i].device_id, m_codes[i].code, false);
          }
        }
      }
      else
      {
        if (m_state)
        {
          // should never happen
        }
        else
        {
          // FIXME: should do in reverse
          for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
          {
            uinput.send_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code, false);
          }
        }
      }

      if (!m_state)
      {
        m_hold_counter = 0;
      }
    }
  }
}

void
KeyButtonEventHandler::update(uInput& uinput, int msec_delta) 
{
  if (m_state && m_hold_threshold)
  {
    if (m_hold_counter < m_hold_threshold &&
        m_hold_counter + msec_delta >= m_hold_threshold)
    {
      // start sending the secondary events
      for(int i = 0; m_secondary_codes[i].is_valid(); ++i)
      {
        uinput.send_key(m_secondary_codes[i].device_id, m_secondary_codes[i].code, true);
      }
      uinput.sync();
    }

    if (m_hold_counter < m_hold_threshold)
    {
      m_hold_counter += msec_delta;
    }
  }
}

std::string
KeyButtonEventHandler::str() const
{
  std::ostringstream out;
  for(int i = 0; m_codes[i].is_valid();)
  {
    out << m_codes[i].device_id << "-" << m_codes[i].code;

    ++i;
    if (m_codes[i].is_valid())
      out << "+";
  }
  return out.str();
}

AbsButtonEventHandler*
AbsButtonEventHandler::from_string(const std::string& str)
{
  // FIXME: Need magic to detect min/max of the axis
  assert(!"not implemented");
}

AbsButtonEventHandler::AbsButtonEventHandler(int code) :
  m_code(UIEvent::invalid()),
  m_value()
{
  assert(!"Not implemented");
  // FIXME: Need magic to detect min/max of the axis
}

void
AbsButtonEventHandler::init(uInput& uinput, int slot, bool extra_devices) const
{
}

void
AbsButtonEventHandler::send(uInput& uinput, bool value)
{
  if (value)
  {
    uinput.send_abs(m_code.device_id, m_code.code, m_value);
  }
}

std::string
AbsButtonEventHandler::str() const
{
  std::ostringstream out;
  out << "abs: " << m_code.device_id << "-" << m_code.code << ":" << m_value; 
  return out.str();
}

RelButtonEventHandler*
RelButtonEventHandler::from_string(const std::string& str)
{
  std::auto_ptr<RelButtonEventHandler> ev;

  int idx = 0;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0:
        ev.reset(new RelButtonEventHandler(str2rel_event(*i)));
        break;

      case 1: 
        ev->m_value  = boost::lexical_cast<int>(*i);
        break;
        
      case 2: 
        ev->m_repeat = boost::lexical_cast<int>(*i); 
        break;
    }
  }

  return ev.release();
}

RelButtonEventHandler::RelButtonEventHandler(const UIEvent& code) :
  m_code(code),
  m_value(3),
  m_repeat(100)
{
}

void
RelButtonEventHandler::init(uInput& uinput, int slot, bool extra_devices) const
{
  uinput.add_rel(m_code.device_id, m_code.code);
}

void
RelButtonEventHandler::send(uInput& uinput, bool value)
{
  if (value)
  {
    uinput.send_rel_repetitive(m_code, m_value, m_repeat);
  }
  else
  {
    uinput.send_rel_repetitive(m_code, m_value, -1);
  } 
}

std::string
RelButtonEventHandler::str() const
{
  std::ostringstream out;
  out << "rel:" << m_code.device_id << "-" << m_code.code << ":" << m_value << ":" << m_repeat;
  return out.str();
}

ExecButtonEventHandler*
ExecButtonEventHandler::from_string(const std::string& str)
{
  std::vector<std::string> args;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  std::copy(tokens.begin(), tokens.end(), std::back_inserter(args));

  return new ExecButtonEventHandler(args);
}

ExecButtonEventHandler::ExecButtonEventHandler(const std::vector<std::string>& args) :
  m_args(args)
{
}

void
ExecButtonEventHandler::init(uInput& uinput, int slot, bool extra_devices) const
{
  // nothing to do
}

void
ExecButtonEventHandler::send(uInput& uinput, bool value)
{
  if (value)
  {
    pid_t pid = fork();
    if (pid == 0)
    {
      char** argv = static_cast<char**>(malloc(sizeof(char*) * (m_args.size() + 1)));
      for(size_t i = 0; i < m_args.size(); ++i)
      {
        argv[i] = strdup(m_args[i].c_str());
      }
      argv[m_args.size()] = NULL;

      if (execvp(m_args[0].c_str(), argv) == -1)
      {
        std::cout << "error: ExecButtonEventHandler::send(): " << strerror(errno) << std::endl;
        _exit(EXIT_FAILURE);
      }
    }
  }
}

std::string
ExecButtonEventHandler::str() const
{
  return "exec";
}

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
MacroButtonEventHandler::init(uInput& uinput, int slot, bool extra_devices) const
{
  for(std::vector<MacroEvent>::const_iterator i = m_events.begin(); i != m_events.end(); ++i)
  {
    switch(i->type)
    {
      case MacroEvent::kSendOp:
        switch(i->send.event.type)
        {
          case EV_REL:
            uinput.add_rel(i->send.event.device_id, i->send.event.code);
            break;

          case EV_KEY:
            uinput.add_key(i->send.event.device_id, i->send.event.code);
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
MacroButtonEventHandler::send(uInput& uinput, bool value)
{
  if (value && !m_send_in_progress)
  {
    m_send_in_progress = true;
    m_event_counter = 0;
    m_countdown = 0;
  }
}

void
MacroButtonEventHandler::update(uInput& uinput, int msec_delta)
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
            uinput.send(m_events[m_event_counter].send.event.device_id,
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
