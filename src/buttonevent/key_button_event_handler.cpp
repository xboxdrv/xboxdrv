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

#include "buttonevent/key_button_event_handler.hpp"

#include <boost/tokenizer.hpp>
#include <linux/input.h>

#include "evdev_helper.hpp"
#include "uinput.hpp"

KeyButtonEventHandler*
KeyButtonEventHandler::from_string(UInput& uinput, int slot, bool extra_devices, 
                                   const std::string& str)
{
  //std::cout << " KeyButtonEventHandler::from_string: " << str << std::endl;
  UIEventSequence codes;
  UIEventSequence secondary_codes;
  int hold_threshold = 0;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++idx)
  {
    switch(idx)
    {
      case 0: 
        {
          codes = UIEventSequence::from_string(*i);
        }
        break;

      case 1:
        {
          secondary_codes = UIEventSequence::from_string(*i);
          hold_threshold = 250;
        }
        break;
        
      case 2:
        {
          hold_threshold = boost::lexical_cast<int>(*i);
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

  return new KeyButtonEventHandler(uinput, slot, extra_devices,
                                   codes, secondary_codes, hold_threshold);
}

KeyButtonEventHandler::KeyButtonEventHandler(UInput& uinput, int slot, bool extra_devices,
                                             const UIEventSequence& codes,
                                             const UIEventSequence& secondary_codes,
                                             int hold_threshold) :
  m_state(false),
  m_codes(codes),
  m_secondary_codes(secondary_codes),
  m_hold_threshold(0),
  m_hold_counter(hold_threshold),
  m_release_scheduled(false)
{
  m_codes.init(uinput, slot, extra_devices);

  if (m_hold_threshold)
  {
    m_secondary_codes.init(uinput, slot, extra_devices);
  }
}

void
KeyButtonEventHandler::send(bool value)
{
  if (m_state != value)
  {
    m_state = value;

    if (m_hold_threshold == 0)
    {
      m_codes.send(m_state);
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
          m_codes.send(true);
          m_release_scheduled = 50;
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
          m_secondary_codes.send(false);
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
KeyButtonEventHandler::update(int msec_delta) 
{
  if (m_release_scheduled)
  {
    m_release_scheduled -= msec_delta;

    if (m_release_scheduled <= 0)
    {
      m_codes.send(false);
      m_release_scheduled = 0;
    }
  }

  if (m_state && m_hold_threshold)
  {
    if (m_hold_counter < m_hold_threshold &&
        m_hold_counter + msec_delta >= m_hold_threshold)
    {
      // start sending the secondary events
      m_secondary_codes.send(true);
      //uinput.sync(); BROKEN: must sync
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
  out << m_codes.str() << ":" << m_secondary_codes.str() << ":" << m_hold_threshold;
  return out.str();
}

/* EOF */
