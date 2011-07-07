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

#include "buttonevent/cycle_key_button_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <stdexcept>
#include <memory>

#include "ui_event_sequence.hpp"
#include "raise_exception.hpp"

std::map<std::string, CycleKeyButtonEventHandler*> CycleKeyButtonEventHandler::s_lookup_table;

CycleKeyButtonEventHandler*
CycleKeyButtonEventHandler::from_string(const std::string& value)
{
  return from_string_named(":" + value);
}

CycleKeyButtonEventHandler*
CycleKeyButtonEventHandler::from_string_named(const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (args.size() < 2)
  {
    raise_exception(std::runtime_error, "need at least two arguments");
  }
  else
  {
    Keys keys;

    std::string name = args[0];
    for(std::vector<std::string>::const_iterator i = args.begin()+1; i != args.end(); ++i)
    {
      keys.push_back(UIEventSequence::from_string(*i));
    }

    std::auto_ptr<CycleKeyButtonEventHandler> cycle(new CycleKeyButtonEventHandler(keys));
 
    // if name is empty, don't put it in the lookup table
    if (!name.empty())
    {
      if (lookup(name) != 0)
      {
        raise_exception(std::runtime_error, "duplicate name entry");
      }

      s_lookup_table.insert(std::pair<std::string, CycleKeyButtonEventHandler*>(name, cycle.get()));
    }

    return cycle.release();
  }
}

CycleKeyButtonEventHandler*
CycleKeyButtonEventHandler::lookup(const std::string& name)
{
  std::map<std::string, CycleKeyButtonEventHandler*>::iterator it = s_lookup_table.find(name);
  if (it == s_lookup_table.end())
  {
    return 0;
  }
  else
  {
    return it->second;
  }
}

CycleKeyButtonEventHandler::CycleKeyButtonEventHandler(const Keys& keys) :
  m_keys(keys),
  m_current_key(keys.size()-1)
{
  assert(!keys.empty());
}

void
CycleKeyButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  for(Keys::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
  {
    i->init(uinput, slot, extra_devices);
  }
}

void
CycleKeyButtonEventHandler::send(UInput& uinput, bool value)
{
  if (value)
  {
    next_key();
    send_only(uinput, value); 
  }
  else
  {
    send_only(uinput, value);
  }
}

void
CycleKeyButtonEventHandler::send_only(UInput& uinput, bool value)
{
  m_keys[m_current_key].send(uinput, value);  
}

void
CycleKeyButtonEventHandler::update(UInput& uinput, int msec_delta)
{
}

void
CycleKeyButtonEventHandler::next_key()
{
  if (m_current_key >= static_cast<int>(m_keys.size())-1)
  {
    m_current_key = 0;
  }
  else
  {
    m_current_key += 1;
  }
}

void
CycleKeyButtonEventHandler::prev_key()
{
  if (m_current_key <= 0)
  {
    m_current_key = m_keys.size()-1;
  }
  else
  {
    m_current_key -= 1;
  }
}

std::string
CycleKeyButtonEventHandler::str() const
{
  return "cycle-key";
}

/* EOF */
