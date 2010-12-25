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

#include <assert.h>
#include <iostream>
#include <linux/input.h>
#include <stdexcept>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "button_event.hpp"
#include "evdev_helper.hpp"
#include "uinput.hpp"
#include "uinput_deviceid.hpp"

ButtonEventPtr
ButtonEvent::invalid()
{
  return ButtonEventPtr();
}

ButtonEventPtr
ButtonEvent::create_abs(int code)
{
  ButtonEventPtr ev(new ButtonEvent);
  ev->m_type  = EV_ABS;
  ev->abs.code  = UIEvent::create(DEVICEID_AUTO, EV_ABS, code);
  ev->abs.value = 1;
  return ev;
}

ButtonEventPtr
ButtonEvent::create_key(int code)
{
  ButtonEventPtr ev(new ButtonEvent);
  ev->m_type = EV_KEY;
  std::fill_n(ev->key.codes, MAX_MODIFIER + 1, UIEvent::invalid());
  ev->key.codes[0] = UIEvent::create(DEVICEID_AUTO, EV_KEY, code);
  return ev;
}

ButtonEventPtr
ButtonEvent::create_key()
{
  ButtonEventPtr ev(new ButtonEvent);
  ev->m_type = EV_KEY;
  std::fill_n(ev->key.codes, MAX_MODIFIER + 1, UIEvent::invalid());
  return ev;
}

ButtonEventPtr
ButtonEvent::create_rel(int code)
{
  ButtonEventPtr ev(new ButtonEvent);
  ev->m_type       = EV_REL;
  ev->rel.code   = UIEvent::create(DEVICEID_AUTO, EV_REL, code);
  ev->rel.value  = 3;
  ev->rel.repeat = 100;
  return ev;
}

ButtonEventPtr
ButtonEvent::from_string(const std::string& str)
{
  ButtonEventPtr ev;

  int j = 0;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    if (j == 0)
    {
      switch(get_event_type(*i))
      {
        case EV_KEY: {
          ev = ButtonEvent::create_key();

          boost::char_separator<char> plus_sep("+", "", boost::keep_empty_tokens);
          tokenizer ev_tokens(*i, plus_sep);
          int k = 0;
          for(tokenizer::iterator m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
          {
            ev->key.codes[k] = str2key_event(*m);
          }
          break;
        }

        case EV_REL:
          ev = ButtonEvent::create_rel(-1); // FIXME: Hack
          ev->rel.code = str2rel_event(*i);
          break;

        case EV_ABS:
          assert(!"Not implemented");
          ev = ButtonEvent::create_abs(-1);
          // FIXME: Need magic to detect min/max of the axis
          break;

        case -1: // void/none
          ev = ButtonEvent::invalid();
          break;

        default:
          assert(!"Unknown type");
      }
    }
    else
    {
      switch (ev->m_type)
      {
        case EV_REL:
          switch(j) {
            case 1: ev->rel.value  = boost::lexical_cast<int>(*i); break;
            case 2: ev->rel.repeat = boost::lexical_cast<int>(*i); break;
          }
          break;

        case EV_ABS:
          break;
      }
    }
  }

  if (false)
    std::cout << "ButtonEvent::from_string():\n  in:  " << str << "\n  out: " << ev->str() << std::endl;

  return ev;
}

ButtonEvent::ButtonEvent() :
  m_type(-1),
  m_filters()
{
}

void
ButtonEvent::init(uInput& uinput) const
{
  switch(m_type)
  {
    case EV_KEY:
      for(int i = 0; key.codes[i].is_valid(); ++i)
      {
        uinput.create_uinput_device(key.codes[i].device_id);
        uinput.add_key(key.codes[i].device_id, key.codes[i].code);
      }
      break;

    case EV_REL:
      uinput.create_uinput_device(rel.code.device_id);
      uinput.get_uinput(rel.code.device_id)->add_rel(rel.code.code);
      break;

    case EV_ABS:
      uinput.create_uinput_device(abs.code.device_id);
      break;

    default:
      std::cout << "ButtonEvent::init(): invalid type: " << m_type << std::endl;
      assert(!"ButtonEvent::init(): never reached");
  }
}

void
ButtonEvent::send(uInput& uinput, bool value) const
{
  for(std::vector<ButtonFilterPtr>::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
  {
    value = (*i)->filter(value);
  }

  switch(m_type)
  {
    case EV_KEY:
      for(int i = 0; key.codes[i].is_valid(); ++i)
      {
        uinput.send_key(key.codes[i].device_id, key.codes[i].code, value);
      }
      break;

    case EV_REL:
      if (value)
      {
        uinput.send_rel_repetitive(rel.code, rel.value, rel.repeat);
      }
      else
      {
        uinput.send_rel_repetitive(rel.code, rel.value, -1);
      }
      break;


    case EV_ABS:
      if (value)
      {
        uinput.get_uinput(abs.code.device_id)->send(EV_ABS, abs.code.code, abs.value);
      }
      break;
  }
}

void
ButtonEvent::set_filters(const std::vector<ButtonFilterPtr>& filters)
{
  m_filters = filters;
}

std::string
ButtonEvent::str() const
{
  std::ostringstream out;
  
  switch(m_type)
  {
    case EV_KEY:
      for(int i = 0; key.codes[i].is_valid();)
      {
        out << key.codes[i].device_id << "-" << key.codes[i].code;

        ++i;
        if (key.codes[i].is_valid())
          out << "+";
      }
      break;

    case EV_REL:
      out << rel.code.device_id << "-" << rel.code.code << ":" << rel.value << ":" << rel.repeat;
      break;

    case EV_ABS:
      out << abs.code.device_id << "-" << abs.code.code << ":" << abs.value;
      break;

    case -1:
      out << "void";
      break;

    default:
      assert(!"Never reached");
  }

  return out.str();
}

/* EOF */
