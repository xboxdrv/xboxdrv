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

#include "ui_event_sequence.hpp"
#include "raise_exception.hpp"

std::map<std::string, CycleKeySequencePtr> CycleKeyButtonEventHandler::s_lookup_table;

namespace {

CycleKeyButtonEventHandler::Direction 
direction_from_string(const std::string& value)
{
  if (value == "forward")
  {
    return CycleKeyButtonEventHandler::kForward;
  }
  else if (value == "backward")
  {
    return CycleKeyButtonEventHandler::kBackward;
  }
  else if (value == "none")
  {
    return CycleKeyButtonEventHandler::kNone;
  }
  else
  {
    raise_exception(std::runtime_error, "allowed values are 'forward', 'backward' and 'none'");
  }
}

} // namespace

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
    std::string name = args[0];
    CycleKeySequencePtr sequence = CycleKeySequence::from_range(args.begin()+1, args.end());

    // if name is empty, don't put it in the lookup table
    if (!name.empty())
    {
      if (lookup(name) != 0)
      {
        raise_exception(std::runtime_error, "duplicate name entry");
      }
      else
      {
        s_lookup_table.insert(std::pair<std::string, CycleKeySequencePtr>(name, sequence));
      }
    }

    return new CycleKeyButtonEventHandler(sequence, kForward, true);
  }
}

CycleKeyButtonEventHandler*
CycleKeyButtonEventHandler::from_string_ref(const std::string& value)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(value, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (args.size() > 0)
  {
    std::string name = args[0];
    Direction direction = (args.size() > 1) ? direction_from_string(args[1]) : kBackward;
    bool press    = (args.size() > 2) ? boost::lexical_cast<bool>(args[2]) : true;

    CycleKeySequencePtr cycle_sequence = CycleKeyButtonEventHandler::lookup(name);
    if (!cycle_sequence)
    {
      raise_exception(std::runtime_error, "unknown cycle sequence: " << name);
    }
    else
    {
      return new CycleKeyButtonEventHandler(cycle_sequence, direction, press);
    }
  }
  else
  {
    raise_exception(std::runtime_error, "need at least one arguments");
  }
}

CycleKeySequencePtr
CycleKeyButtonEventHandler::lookup(const std::string& name)
{
  std::map<std::string, CycleKeySequencePtr>::iterator it = s_lookup_table.find(name);
  if (it == s_lookup_table.end())
  {
    return CycleKeySequencePtr();
  }
  else
  {
    return it->second;
  }
}

CycleKeyButtonEventHandler::CycleKeyButtonEventHandler(CycleKeySequencePtr sequence,
                                                       Direction direction, 
                                                       bool send_press) :
  m_sequence(sequence),
  m_direction(direction),
  m_send_press(send_press)
{
}

void
CycleKeyButtonEventHandler::init(UInput& uinput, int slot, bool extra_devices)
{
  // CycleKeySequence will make sure that init() is only called once
  m_sequence->init(uinput, slot, extra_devices);
}

void
CycleKeyButtonEventHandler::send(UInput& uinput, bool value)
{
  if (value)
  {
    if (m_send_press && m_sequence->has_current_key())
    {
      m_sequence->send(uinput, value);
    }
    else
    {
      switch(m_direction)
      {
        case kBackward:
          m_sequence->prev_key();
          break;

        case kForward:
          m_sequence->next_key();
          break;

        case kNone:
          break;
      }

      if (m_send_press)
      {
        m_sequence->send(uinput, value);
      }
    }
  }
  else
  {
    if (m_send_press)
    {
      m_sequence->send(uinput, value);
    }
  }
}

void
CycleKeyButtonEventHandler::update(UInput& uinput, int msec_delta)
{
}

std::string
CycleKeyButtonEventHandler::str() const
{
  return "cycle-key";
}

/* EOF */
