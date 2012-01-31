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

#include "ui_event_sequence.hpp"

#include <boost/tokenizer.hpp>

#include "evdev_helper.hpp"
#include "ui_event.hpp"
#include "uinput.hpp"

UIEventSequence
UIEventSequence::from_string(const std::string& value)
{
  UIEvents sequence;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer ev_tokens(value, boost::char_separator<char>("+", "", boost::keep_empty_tokens));
  int k = 0;
  for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
  {
    sequence.push_back(str2key_event(*m));
  }

  return sequence;
}

UIEventSequence
UIEventSequence::from_char(char c)
{
  switch(c)
  {
    case 'A': return from_string("XK_Shift_L+XK_a");
    case 'B': return from_string("XK_Shift_L+XK_b"); 
    case 'C': return from_string("XK_Shift_L+XK_c"); 
    case 'D': return from_string("XK_Shift_L+XK_d"); 
    case 'E': return from_string("XK_Shift_L+XK_e"); 
    case 'F': return from_string("XK_Shift_L+XK_f"); 
    case 'G': return from_string("XK_Shift_L+XK_g"); 
    case 'H': return from_string("XK_Shift_L+XK_h"); 
    case 'I': return from_string("XK_Shift_L+XK_i"); 
    case 'J': return from_string("XK_Shift_L+XK_j"); 
    case 'K': return from_string("XK_Shift_L+XK_k"); 
    case 'L': return from_string("XK_Shift_L+XK_l"); 
    case 'M': return from_string("XK_Shift_L+XK_m"); 
    case 'N': return from_string("XK_Shift_L+XK_n"); 
    case 'O': return from_string("XK_Shift_L+XK_o"); 
    case 'P': return from_string("XK_Shift_L+XK_p"); 
    case 'Q': return from_string("XK_Shift_L+XK_q"); 
    case 'R': return from_string("XK_Shift_L+XK_r"); 
    case 'S': return from_string("XK_Shift_L+XK_s"); 
    case 'T': return from_string("XK_Shift_L+XK_t"); 
    case 'U': return from_string("XK_Shift_L+XK_u"); 
    case 'V': return from_string("XK_Shift_L+XK_v"); 
    case 'W': return from_string("XK_Shift_L+XK_w"); 
    case 'X': return from_string("XK_Shift_L+XK_x"); 
    case 'Y': return from_string("XK_Shift_L+XK_y"); 
    case 'Z': return from_string("XK_Shift_L+XK_z"); 

    case 'a': return from_string("XK_a");
    case 'b': return from_string("XK_b"); 
    case 'c': return from_string("XK_c"); 
    case 'd': return from_string("XK_d"); 
    case 'e': return from_string("XK_e"); 
    case 'f': return from_string("XK_f"); 
    case 'g': return from_string("XK_g"); 
    case 'h': return from_string("XK_h"); 
    case 'i': return from_string("XK_i"); 
    case 'j': return from_string("XK_j"); 
    case 'k': return from_string("XK_k"); 
    case 'l': return from_string("XK_l"); 
    case 'm': return from_string("XK_m"); 
    case 'n': return from_string("XK_n"); 
    case 'o': return from_string("XK_o"); 
    case 'p': return from_string("XK_p"); 
    case 'q': return from_string("XK_q"); 
    case 'r': return from_string("XK_r"); 
    case 's': return from_string("XK_s"); 
    case 't': return from_string("XK_t"); 
    case 'u': return from_string("XK_u"); 
    case 'v': return from_string("XK_v"); 
    case 'w': return from_string("XK_w"); 
    case 'x': return from_string("XK_x"); 
    case 'y': return from_string("XK_y"); 
    case 'z': return from_string("XK_z"); 

    case '0': return from_string("XK_0"); 
    case '1': return from_string("XK_1"); 
    case '2': return from_string("XK_2"); 
    case '3': return from_string("XK_3"); 
    case '4': return from_string("XK_4"); 
    case '5': return from_string("XK_5"); 
    case '6': return from_string("XK_6"); 
    case '7': return from_string("XK_7"); 
    case '8': return from_string("XK_8"); 
    case '9': return from_string("XK_9"); 
    case '.': return from_string("XK_period"); 
    case '\n': return from_string("XK_enter"); 

    default:  return from_string("XK_space"); break;
  }
}

UIEventSequence::UIEventSequence() :
  m_sequence(),
  m_emitters()
{
}

UIEventSequence::UIEventSequence(const UIEvents& sequence) :
  m_sequence(sequence),
  m_emitters()
{
}

UIEventSequence::UIEventSequence(const UIEvent& event) :
  m_sequence(1, event),
  m_emitters()
{
}

void
UIEventSequence::init(UInput& uinput, int slot, bool extra_devices)
{
  for(UIEvents::iterator i = m_sequence.begin(); i != m_sequence.end(); ++i)
  {
    i->resolve_device_id(slot, extra_devices);
    m_emitters.push_back(uinput.add_key(i->get_device_id(), i->code));
  }
}

void
UIEventSequence::send(int value)
{
  if (value)
  {
    for(UIEventEmitters::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
    {
      (*i)->send(value);
    }
  }
  else
  {
    // on release, send events in reverse order
    for(UIEventEmitters::reverse_iterator i = m_emitters.rbegin(); i != m_emitters.rend(); ++i)
    { 
      (*i)->send(value);
    }
  }
}

void
UIEventSequence::clear()
{
  m_sequence.clear();
}

std::string
UIEventSequence::str() const
{
  std::ostringstream out;

  for(UIEvents::const_iterator i = m_sequence.begin(); i != m_sequence.end(); ++i)
  {
    out << i->get_device_id() << "-" << i->code;

    if (i != m_sequence.end()-1)
      out << "+";
  }

  return out.str();
}

/* EOF */
