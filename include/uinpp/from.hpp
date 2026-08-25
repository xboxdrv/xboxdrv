// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_NPP_FROM_HPP
#define HEADER_NPP_FROM_HPP

#include <linux/input.h>
#include <vector>

#include <strut/split.hpp>
#include <uinpp/event.hpp>
#include <uinpp/event_sequence.hpp>

#include "evdev_helper.hpp"

namespace uinpp {

inline
Event
Event_from_char(char c)
{
  Event ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_AUTO;
  ev.m_device_id_resolved = false;
  ev.type = EV_KEY;

  // FIXME: not very complete, should use EventSequence and do a bit more magic
  switch(c)
  {
    case 'a': ev.code = str2key("XK_a"); break;
    case 'b': ev.code = str2key("XK_b"); break;
    case 'c': ev.code = str2key("XK_c"); break;
    case 'd': ev.code = str2key("XK_d"); break;
    case 'e': ev.code = str2key("XK_e"); break;
    case 'f': ev.code = str2key("XK_f"); break;
    case 'g': ev.code = str2key("XK_g"); break;
    case 'h': ev.code = str2key("XK_h"); break;
    case 'i': ev.code = str2key("XK_i"); break;
    case 'j': ev.code = str2key("XK_j"); break;
    case 'k': ev.code = str2key("XK_k"); break;
    case 'l': ev.code = str2key("XK_l"); break;
    case 'm': ev.code = str2key("XK_m"); break;
    case 'n': ev.code = str2key("XK_n"); break;
    case 'o': ev.code = str2key("XK_o"); break;
    case 'p': ev.code = str2key("XK_p"); break;
    case 'q': ev.code = str2key("XK_q"); break;
    case 'r': ev.code = str2key("XK_r"); break;
    case 's': ev.code = str2key("XK_s"); break;
    case 't': ev.code = str2key("XK_t"); break;
    case 'u': ev.code = str2key("XK_u"); break;
    case 'v': ev.code = str2key("XK_v"); break;
    case 'w': ev.code = str2key("XK_w"); break;
    case 'x': ev.code = str2key("XK_x"); break;
    case 'y': ev.code = str2key("XK_y"); break;
    case 'z': ev.code = str2key("XK_z"); break;
    case '0': ev.code = str2key("XK_0"); break;
    case '1': ev.code = str2key("XK_1"); break;
    case '2': ev.code = str2key("XK_2"); break;
    case '3': ev.code = str2key("XK_3"); break;
    case '4': ev.code = str2key("XK_4"); break;
    case '5': ev.code = str2key("XK_5"); break;
    case '6': ev.code = str2key("XK_6"); break;
    case '7': ev.code = str2key("XK_7"); break;
    case '8': ev.code = str2key("XK_8"); break;
    case '9': ev.code = str2key("XK_9"); break;

    case '.': ev.code = str2key("XK_period"); break;

    case '\n': ev.code = str2key("XK_enter"); break;

    default:  ev.code = str2key("XK_space"); break;
  }
  return ev;
}

inline
Event
Event_from_string(std::string const& str)
{
  switch(get_event_type(str))
  {
    case EV_REL: return str2rel_event(str); break;
    case EV_ABS: return str2abs_event(str); break;
    case EV_KEY: return str2key_event(str); break;
    default: throw std::runtime_error("unknown event type");
  }
}

inline
EventSequence
EventSequence_from_string(std::string const& value)
{
  std::vector<Event> sequence;

  auto ev_tokens = strut::split(value, '+');
  int k = 0;
  for(auto m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
  {
    sequence.push_back(str2key_event(*m));
  }

  return sequence;
}

inline
EventSequence
EventSequence_from_char(char c)
{
  switch(c)
  {
    case 'A': return EventSequence_from_string("XK_Shift_L+XK_a");
    case 'B': return EventSequence_from_string("XK_Shift_L+XK_b");
    case 'C': return EventSequence_from_string("XK_Shift_L+XK_c");
    case 'D': return EventSequence_from_string("XK_Shift_L+XK_d");
    case 'E': return EventSequence_from_string("XK_Shift_L+XK_e");
    case 'F': return EventSequence_from_string("XK_Shift_L+XK_f");
    case 'G': return EventSequence_from_string("XK_Shift_L+XK_g");
    case 'H': return EventSequence_from_string("XK_Shift_L+XK_h");
    case 'I': return EventSequence_from_string("XK_Shift_L+XK_i");
    case 'J': return EventSequence_from_string("XK_Shift_L+XK_j");
    case 'K': return EventSequence_from_string("XK_Shift_L+XK_k");
    case 'L': return EventSequence_from_string("XK_Shift_L+XK_l");
    case 'M': return EventSequence_from_string("XK_Shift_L+XK_m");
    case 'N': return EventSequence_from_string("XK_Shift_L+XK_n");
    case 'O': return EventSequence_from_string("XK_Shift_L+XK_o");
    case 'P': return EventSequence_from_string("XK_Shift_L+XK_p");
    case 'Q': return EventSequence_from_string("XK_Shift_L+XK_q");
    case 'R': return EventSequence_from_string("XK_Shift_L+XK_r");
    case 'S': return EventSequence_from_string("XK_Shift_L+XK_s");
    case 'T': return EventSequence_from_string("XK_Shift_L+XK_t");
    case 'U': return EventSequence_from_string("XK_Shift_L+XK_u");
    case 'V': return EventSequence_from_string("XK_Shift_L+XK_v");
    case 'W': return EventSequence_from_string("XK_Shift_L+XK_w");
    case 'X': return EventSequence_from_string("XK_Shift_L+XK_x");
    case 'Y': return EventSequence_from_string("XK_Shift_L+XK_y");
    case 'Z': return EventSequence_from_string("XK_Shift_L+XK_z");

    case 'a': return EventSequence_from_string("XK_a");
    case 'b': return EventSequence_from_string("XK_b");
    case 'c': return EventSequence_from_string("XK_c");
    case 'd': return EventSequence_from_string("XK_d");
    case 'e': return EventSequence_from_string("XK_e");
    case 'f': return EventSequence_from_string("XK_f");
    case 'g': return EventSequence_from_string("XK_g");
    case 'h': return EventSequence_from_string("XK_h");
    case 'i': return EventSequence_from_string("XK_i");
    case 'j': return EventSequence_from_string("XK_j");
    case 'k': return EventSequence_from_string("XK_k");
    case 'l': return EventSequence_from_string("XK_l");
    case 'm': return EventSequence_from_string("XK_m");
    case 'n': return EventSequence_from_string("XK_n");
    case 'o': return EventSequence_from_string("XK_o");
    case 'p': return EventSequence_from_string("XK_p");
    case 'q': return EventSequence_from_string("XK_q");
    case 'r': return EventSequence_from_string("XK_r");
    case 's': return EventSequence_from_string("XK_s");
    case 't': return EventSequence_from_string("XK_t");
    case 'u': return EventSequence_from_string("XK_u");
    case 'v': return EventSequence_from_string("XK_v");
    case 'w': return EventSequence_from_string("XK_w");
    case 'x': return EventSequence_from_string("XK_x");
    case 'y': return EventSequence_from_string("XK_y");
    case 'z': return EventSequence_from_string("XK_z");

    case '0': return EventSequence_from_string("XK_0");
    case '1': return EventSequence_from_string("XK_1");
    case '2': return EventSequence_from_string("XK_2");
    case '3': return EventSequence_from_string("XK_3");
    case '4': return EventSequence_from_string("XK_4");
    case '5': return EventSequence_from_string("XK_5");
    case '6': return EventSequence_from_string("XK_6");
    case '7': return EventSequence_from_string("XK_7");
    case '8': return EventSequence_from_string("XK_8");
    case '9': return EventSequence_from_string("XK_9");
    case '.': return EventSequence_from_string("XK_period");
    case '\n': return EventSequence_from_string("XK_enter");

    default:  return EventSequence_from_string("XK_space"); break;
  }
}

} // namespace uinpp

#endif

/* EOF */
