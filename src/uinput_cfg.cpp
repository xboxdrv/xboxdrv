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

#include <boost/tokenizer.hpp>
#include <iostream>
#include <linux/input.h>

#include "uinput_cfg.hpp"

uInputCfg::uInputCfg() :
  device_name("Xbox Gamepad (userspace driver)"),
  force_feedback(false),
  config_toggle_button(XBOX_BTN_UNKNOWN),
  map(),
  current_input_map(0)
{
  map.push_back(InputMapping());
  set_defaults();
}

ButtonMap&
uInputCfg::get_btn_map()
{
  return map[current_input_map].btn_map;
}

AxisMap& 
uInputCfg::get_axis_map()
{
  return map[current_input_map].axis_map;
}

ButtonMap&
uInputCfg::get_btn_map(int n)
{
  return map[n].btn_map;
}

AxisMap&
uInputCfg::get_axis_map(int n)
{
  return map[n].axis_map;
}

void
uInputCfg::add_input_mapping()
{
  map.push_back(InputMapping());
  current_input_map = map.size()-1;
  set_defaults();
}

void
uInputCfg::next_input_mapping()
{
  current_input_map += 1;
  if (current_input_map >= static_cast<int>(map.size()))
    current_input_map = 0;
  std::cout << "uInputCfg::next_input_mapping(): " << current_input_map << std::endl;
}

void
uInputCfg::mimic_xpad()
{
  device_name = "Microsoft X-Box 360 pad";

  get_btn_map().bind(XBOX_BTN_START, ButtonEvent::create_key(BTN_START));
  get_btn_map().bind(XBOX_BTN_GUIDE, ButtonEvent::create_key(BTN_MODE));
  get_btn_map().bind(XBOX_BTN_BACK, ButtonEvent::create_key(BTN_BACK));

  get_btn_map().bind(XBOX_BTN_A, ButtonEvent::create_key(BTN_A));
  get_btn_map().bind(XBOX_BTN_B, ButtonEvent::create_key(BTN_B));
  get_btn_map().bind(XBOX_BTN_X, ButtonEvent::create_key(BTN_X));
  get_btn_map().bind(XBOX_BTN_Y, ButtonEvent::create_key(BTN_Y));

  get_btn_map().bind(XBOX_BTN_GREEN, ButtonEvent::create_key(BTN_0));
  get_btn_map().bind(XBOX_BTN_RED, ButtonEvent::create_key(BTN_1));
  get_btn_map().bind(XBOX_BTN_YELLOW, ButtonEvent::create_key(BTN_2));
  get_btn_map().bind(XBOX_BTN_BLUE, ButtonEvent::create_key(BTN_3));
  get_btn_map().bind(XBOX_BTN_ORANGE, ButtonEvent::create_key(BTN_4));

  get_btn_map().bind(XBOX_BTN_WHITE, ButtonEvent::create_key(BTN_TL));
  get_btn_map().bind(XBOX_BTN_BLACK, ButtonEvent::create_key(BTN_TR));

  get_btn_map().bind(XBOX_BTN_LB, ButtonEvent::create_key(BTN_TL));
  get_btn_map().bind(XBOX_BTN_RB, ButtonEvent::create_key(BTN_TR));
            
  get_btn_map().bind(XBOX_BTN_LT, ButtonEvent::create_key(BTN_TL2));
  get_btn_map().bind(XBOX_BTN_RT, ButtonEvent::create_key(BTN_TR2));
            
  get_btn_map().bind(XBOX_BTN_THUMB_L, ButtonEvent::create_key(BTN_THUMBL));
  get_btn_map().bind(XBOX_BTN_THUMB_R, ButtonEvent::create_key(BTN_THUMBR));
            
  get_btn_map().bind(XBOX_DPAD_UP, ButtonEvent::create_key(BTN_BASE));
  get_btn_map().bind(XBOX_DPAD_DOWN, ButtonEvent::create_key(BTN_BASE2));
  get_btn_map().bind(XBOX_DPAD_LEFT, ButtonEvent::create_key(BTN_BASE3));
  get_btn_map().bind(XBOX_DPAD_RIGHT, ButtonEvent::create_key(BTN_BASE4));

  // Axis Mapping
  get_axis_map().bind(XBOX_AXIS_X1,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_X,  -32768, 32767, 16, 128));
  get_axis_map().bind(XBOX_AXIS_Y1,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_Y,  -32768, 32767, 16, 128));
  get_axis_map().bind(XBOX_AXIS_X2,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_RX, -32768, 32767, 16, 128));
  get_axis_map().bind(XBOX_AXIS_Y2,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_RY, -32768, 32767, 16, 128));
  get_axis_map().bind(XBOX_AXIS_LT,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z,  0, 255, 0, 0));
  get_axis_map().bind(XBOX_AXIS_RT,      AxisEvent::create_abs(DEVICEID_AUTO, ABS_RZ, 0, 255, 0, 0));
  get_axis_map().bind(XBOX_AXIS_TRIGGER, AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z, -255, 255, 0, 0));
  get_axis_map().bind(XBOX_AXIS_DPAD_X,  AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0X, -1, 1, 0, 0));
  get_axis_map().bind(XBOX_AXIS_DPAD_Y,  AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0Y, -1, 1, 0, 0));
}

void
uInputCfg::mouse()
{
  get_axis_map().clear();
  get_btn_map().clear();

  // send events only every 20msec, lower values cause a jumpy pointer
  set_ui_axismap("x1^dead:4000", "REL_X:15:20");
  set_ui_axismap("y1^dead:4000", "REL_Y:15:20");
  set_ui_axismap("tr+x1^dead:4000^resp:-8000:0:8000", "REL_X:15:20");
  set_ui_axismap("tr+y1^dead:4000^resp:-8000:0:8000", "REL_Y:15:20");
  set_ui_axismap("y2^invert^dead:6000", "REL_WHEEL:5:100");
  set_ui_axismap("x2^dead:6000", "REL_HWHEEL:5:100");
  set_ui_axismap("trigger^invert", "REL_WHEEL:5:100");
        
  set_ui_buttonmap("a", "BTN_LEFT");
  set_ui_buttonmap("b", "BTN_RIGHT");
  set_ui_buttonmap("x", "BTN_MIDDLE");
  set_ui_buttonmap("y", "KEY_ENTER");
  set_ui_buttonmap("rb", "KEY_PAGEDOWN");
  set_ui_buttonmap("lb", "KEY_PAGEUP");
        
  set_ui_buttonmap("dl", "KEY_LEFT");
  set_ui_buttonmap("dr", "KEY_RIGHT");
  set_ui_buttonmap("du", "KEY_UP");
  set_ui_buttonmap("dd", "KEY_DOWN");
        
  set_ui_buttonmap("start", "KEY_FORWARD");
  set_ui_buttonmap("back", "KEY_BACK");
  set_ui_buttonmap("guide", "KEY_ESC");
}

void
uInputCfg::set_defaults()
{
  get_btn_map().clear();
  get_axis_map().clear();

  // Button Mapping
  get_btn_map().bind(XBOX_BTN_START, ButtonEvent::create_key(BTN_START));
  get_btn_map().bind(XBOX_BTN_GUIDE, ButtonEvent::create_key(BTN_MODE));
  get_btn_map().bind(XBOX_BTN_BACK, ButtonEvent::create_key(BTN_SELECT));

  get_btn_map().bind(XBOX_BTN_A, ButtonEvent::create_key(BTN_A));
  get_btn_map().bind(XBOX_BTN_B, ButtonEvent::create_key(BTN_B));
  get_btn_map().bind(XBOX_BTN_X, ButtonEvent::create_key(BTN_X));
  get_btn_map().bind(XBOX_BTN_Y, ButtonEvent::create_key(BTN_Y));

  get_btn_map().bind(XBOX_BTN_GREEN, ButtonEvent::create_key(BTN_0));
  get_btn_map().bind(XBOX_BTN_RED, ButtonEvent::create_key(BTN_1));
  get_btn_map().bind(XBOX_BTN_YELLOW, ButtonEvent::create_key(BTN_2));
  get_btn_map().bind(XBOX_BTN_BLUE, ButtonEvent::create_key(BTN_3));
  get_btn_map().bind(XBOX_BTN_ORANGE, ButtonEvent::create_key(BTN_4));

  get_btn_map().bind(XBOX_BTN_WHITE, ButtonEvent::create_key(BTN_TL));
  get_btn_map().bind(XBOX_BTN_BLACK, ButtonEvent::create_key(BTN_TR));

  get_btn_map().bind(XBOX_BTN_LB, ButtonEvent::create_key(BTN_TL));
  get_btn_map().bind(XBOX_BTN_RB, ButtonEvent::create_key(BTN_TR));

  // by default unmapped:
  //get_btn_map().bind(XBOX_BTN_LT, ButtonEvent::create_key(BTN_TL2));
  //get_btn_map().bind(XBOX_BTN_RT, ButtonEvent::create_key(BTN_TR2));

  get_btn_map().bind(XBOX_BTN_THUMB_L, ButtonEvent::create_key(BTN_THUMBL));
  get_btn_map().bind(XBOX_BTN_THUMB_R, ButtonEvent::create_key(BTN_THUMBR));

  // by default unmapped
  //get_btn_map().bind(XBOX_DPAD_UP,    ButtonEvent::create_key(BTN_BASE));
  //get_btn_map().bind(XBOX_DPAD_DOWN,  ButtonEvent::create_key(BTN_BASE2));
  //get_btn_map().bind(XBOX_DPAD_LEFT,  ButtonEvent::create_key(BTN_BASE3));
  //get_btn_map().bind(XBOX_DPAD_RIGHT, ButtonEvent::create_key(BTN_BASE4));

  // Axis Mapping
  get_axis_map().bind(XBOX_AXIS_X1, AxisEvent::create_abs(DEVICEID_AUTO, ABS_X, -32768, 32767, 0, 0));
  get_axis_map().bind(XBOX_AXIS_Y1, AxisEvent::create_abs(DEVICEID_AUTO, ABS_Y, -32768, 32767, 0, 0));
  get_axis_map().bind(XBOX_AXIS_X2, AxisEvent::create_abs(DEVICEID_AUTO, ABS_RX, -32768, 32767, 0, 0));
  get_axis_map().bind(XBOX_AXIS_Y2, AxisEvent::create_abs(DEVICEID_AUTO, ABS_RY, -32768, 32767, 0, 0));
  get_axis_map().bind(XBOX_AXIS_LT, AxisEvent::create_abs(DEVICEID_AUTO, ABS_BRAKE, 0, 255, 0, 0));
  get_axis_map().bind(XBOX_AXIS_RT, AxisEvent::create_abs(DEVICEID_AUTO, ABS_GAS, 0, 255, 0, 0)); 

  // by default unmapped:
  //get_axis_map().bind(XBOX_AXIS_TRIGGER,  AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z, -255, 255, 0, 0));
  
  get_axis_map().bind(XBOX_AXIS_DPAD_X, AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0X, -1, 1, 0, 0));
  get_axis_map().bind(XBOX_AXIS_DPAD_Y, AxisEvent::create_abs(DEVICEID_AUTO, ABS_HAT0Y, -1, 1, 0, 0));
}

void
uInputCfg::trigger_as_button()
{
  get_axis_map().bind(XBOX_AXIS_LT, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_RT, AxisEvent::invalid());
  get_btn_map().bind(XBOX_BTN_LT, ButtonEvent::create_key(BTN_TL2));
  get_btn_map().bind(XBOX_BTN_RT, ButtonEvent::create_key(BTN_TR2));
}

void
uInputCfg::trigger_as_zaxis()
{
  get_axis_map().bind(XBOX_AXIS_TRIGGER, AxisEvent::create_abs(DEVICEID_AUTO, ABS_Z, -255, 255, 0, 0));
  get_axis_map().bind(XBOX_AXIS_LT, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_RT, AxisEvent::invalid());
}

void
uInputCfg::dpad_as_button()
{
  get_btn_map().bind(XBOX_DPAD_UP,    ButtonEvent::create_key(BTN_BASE));
  get_btn_map().bind(XBOX_DPAD_DOWN,  ButtonEvent::create_key(BTN_BASE2));
  get_btn_map().bind(XBOX_DPAD_LEFT,  ButtonEvent::create_key(BTN_BASE3));
  get_btn_map().bind(XBOX_DPAD_RIGHT, ButtonEvent::create_key(BTN_BASE4));

  get_axis_map().bind(XBOX_AXIS_DPAD_X, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_DPAD_Y, AxisEvent::invalid());
}

void
uInputCfg::dpad_only()
{
  get_axis_map().bind(XBOX_AXIS_X1, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_Y1, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_X2, AxisEvent::invalid());
  get_axis_map().bind(XBOX_AXIS_Y2, AxisEvent::invalid());

  get_axis_map().bind(XBOX_AXIS_DPAD_X, AxisEvent::create_abs(DEVICEID_AUTO, ABS_X, -1, 1, 0, 0));
  get_axis_map().bind(XBOX_AXIS_DPAD_Y, AxisEvent::create_abs(DEVICEID_AUTO, ABS_Y, -1, 1, 0, 0));
}

void
uInputCfg::set_ui_axismap(const std::string& name, const std::string& value)
{
  AxisEventPtr event;

  XboxButton shift = XBOX_BTN_UNKNOWN;
  XboxAxis   axis  = XBOX_AXIS_UNKNOWN;
  std::vector<AxisFilterPtr> filters;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(name, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    { 
      case 0: // shift+key portion
        {
          std::string::size_type j = t->find('+');
          if (j == std::string::npos)
          {
            shift = XBOX_BTN_UNKNOWN;
            axis  = string2axis(*t);
          }
          else
          {
            shift = string2btn(t->substr(0, j));
            axis  = string2axis(t->substr(j+1));
          }
          
          if (value.empty())
          { // if no rhs value is given, add filters to the current binding
            event = get_axis_map().lookup(shift, axis);
          }
          else
          {
            event = AxisEvent::from_string(value);
            if (event)
            {
              if (axis != XBOX_AXIS_UNKNOWN)
              {
                event->set_axis_range(get_axis_min(axis),
                                      get_axis_max(axis));
              }

              get_axis_map().bind(shift, axis, event);
            }
          }
        }
        break;

      default:
        { // filter
          if (event)
          {
            event->add_filter(AxisFilter::from_string(*t));
          }
        }
        break;
    }
  }
}

void
uInputCfg::set_ui_buttonmap(const std::string& name, const std::string& value)
{
  ButtonEventPtr event;

  XboxButton shift = XBOX_BTN_UNKNOWN;
  XboxButton btn   = XBOX_BTN_UNKNOWN;
  std::vector<ButtonFilterPtr> filters;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(name, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    { 
      case 0: // shift+key portion
        {
          std::string::size_type j = t->find('+');
          if (j == std::string::npos)
          {
            shift = XBOX_BTN_UNKNOWN;
            btn   = string2btn(*t);
          }
          else
          {
            shift = string2btn(t->substr(0, j));
            btn   = string2btn(t->substr(j+1));
          }
          
          if (value.empty())
          { // if no rhs value is given, add filters to the current binding
            event = get_btn_map().lookup(shift, btn);
          }
          else
          {
            event = ButtonEvent::from_string(value);
            if (event)
            {
              get_btn_map().bind(shift, btn, event);
            }
          }
        }
        break;

      default:
        { // filter
          if (event)
          {
            event->add_filter(ButtonFilter::from_string(*t));
          }
        }
        break;
    }
  }
}

/* EOF */
