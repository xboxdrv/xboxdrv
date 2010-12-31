/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <math.h>

#include "options.hpp"
#include "helper.hpp"
#include "modifier.hpp"

void apply_button_map(XboxGenericMsg& msg, const std::vector<ButtonMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<ButtonMapping>::const_iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->lhs, 0);

  for(std::vector<ButtonMapping>::const_iterator i = lst.begin(); i != lst.end(); ++i)
    set_button(newmsg, i->rhs, get_button(msg, i->lhs) || get_button(newmsg, i->rhs));

  msg = newmsg;  
}

void apply_axis_map(XboxGenericMsg& msg, const std::vector<AxisMapping>& lst)
{
  XboxGenericMsg newmsg = msg;

  for(std::vector<AxisMapping>::const_iterator i = lst.begin(); i != lst.end(); ++i)
  {
    set_axis_float(newmsg, i->lhs, 0);
  }

  for(std::vector<AxisMapping>::const_iterator i = lst.begin(); i != lst.end(); ++i)
  {
    float lhs  = get_axis_float(msg,    i->lhs);
    float nrhs = get_axis_float(newmsg, i->rhs);

    if (i->invert)
    {
      lhs = -lhs;
    }

    set_axis_float(newmsg, i->rhs, std::max(std::min(nrhs + lhs, 1.0f), -1.0f));
  }
  msg = newmsg;
}

CalibrationMapping CalibrationMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  CalibrationMapping mapping; 
  mapping.axis    = string2axis(lhs);
  mapping.min     = -32768;
  mapping.center  = 0;
  mapping.max     = 32767;

  boost::char_separator<char> sep(":", "", boost::keep_empty_tokens);
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  tokenizer tokens(rhs, sep);
  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    //std::cout << "Token: '" << *i << "'" << std::endl;

    if (!i->empty())
    {
      try 
      {
        if (j == 0) 
          mapping.min = boost::lexical_cast<int>(*i);
        else if (j == 1)
          mapping.center = boost::lexical_cast<int>(*i);
        else if (j == 2)
          mapping.max = boost::lexical_cast<int>(*i);
        else 
          throw std::runtime_error("--calibration: to many arguments given, syntax is 'AXIS=MIN:CENTER:MAX'");
      }
      catch(boost::bad_lexical_cast&) 
      {
        throw std::runtime_error("--calibration: couldn't convert '" + *i + "' to int");
      }
    }
  }
      
  if (!(mapping.min <= mapping.center && mapping.center <= mapping.max))
    throw std::runtime_error("Order wrong 'AXIS=MIN:CENTER:MAX'");

  return mapping;
}

static int clamp(int lhs, int rhs, int v)
{
  return std::max(lhs, std::min(v, rhs));
}

void apply_calibration_map(XboxGenericMsg& msg, const std::vector<CalibrationMapping>& lst)
{
  for(std::vector<CalibrationMapping>::const_iterator i = lst.begin(); i != lst.end(); ++i)
  {
    int value = get_axis(msg,  i->axis);

    if (value < i->center)
      value = 32768 * (value - i->center) / (i->center - i->min);
    else if (value > i->center)
      value = 32767 * (value - i->center) / (i->max - i->center);
    else
      value = 0;

    set_axis(msg, i->axis, clamp(-32768, 32767, value));
  }
}

ButtonMapping 
ButtonMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  ButtonMapping mapping;
  mapping.lhs = string2btn(lhs);
  mapping.rhs = string2btn(rhs);
  return mapping;
}

AxisMapping
AxisMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  assert(!lhs.empty());
  assert(!rhs.empty());

  AxisMapping mapping;

  if (lhs[0] == '-')
  {
    mapping.invert = true;
    mapping.lhs = string2axis(lhs.substr(1));
  }
  else
  {
    mapping.invert = false;
    mapping.lhs = string2axis(lhs);
  }

  mapping.rhs = string2axis(rhs);

  if (mapping.lhs == XBOX_AXIS_UNKNOWN ||
      mapping.rhs == XBOX_AXIS_UNKNOWN)
  {
    throw std::runtime_error("Couldn't convert string \"" + lhs + "=" + rhs + "\" to axis mapping");
  }

  return mapping;
}

RelativeAxisMapping
RelativeAxisMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  /* Format of str: A={SPEED} */
  RelativeAxisMapping mapping;
  mapping.axis  = string2axis(lhs);
  mapping.speed = boost::lexical_cast<int>(rhs);
  // FIXME: insert some error checking here
  return mapping;
}

AutoFireMapping 
AutoFireMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  /* Format of str: A={ON-DELAY}[:{OFF-DELAY}]
     Examples: A=10 or A=10:50 
     if OFF-DELAY == nil then ON-DELAY = OFF-DELAY 
  */
  AutoFireMapping mapping; 
  mapping.button    = string2btn(lhs);
  mapping.frequency = boost::lexical_cast<int>(rhs);
  return mapping;
}

AxisSensitivityMapping 
AxisSensitivityMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  /* 
     Format of str: X1=SENSITIVITY
     Example: X1=2.0
  */
  AxisSensitivityMapping mapping;
  mapping.axis = string2axis(lhs);
  mapping.sensitivity = boost::lexical_cast<float>(rhs);
  return mapping;
}

void squarify_axis_(int16_t& x_inout, int16_t& y_inout)
{
  if (x_inout != 0 || y_inout != 0)
  {
    // Convert values to float
    float x = (x_inout < 0) ? static_cast<float>(x_inout) / 32768.0f : static_cast<float>(x_inout) / 32767.0f;
    float y = (y_inout < 0) ? static_cast<float>(y_inout) / 32768.0f : static_cast<float>(y_inout) / 32767.0f;

    // Transform values to square range
    float l = sqrtf(x*x + y*y);
    float v = fabsf((fabsf(x) > fabsf(y)) ? l/x : l/y);
    x *= v;
    y *= v;

    // Convert values to int16_t
    x_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((x < 0) ? x * 32768 : x * 32767), 32767));
    y_inout = static_cast<int16_t>(Math::clamp(-32768, static_cast<int>((y < 0) ? y * 32768 : y * 32767), 32767));
  }
}

// Little hack to allow access to bitfield via reference
#define squarify_axis(x, y)                     \
  {                                             \
    int16_t x_ = x;                             \
    int16_t y_ = y;                             \
    squarify_axis_(x_, y_);                     \
    x = x_;                                     \
    y = y_;                                     \
  }

void apply_square_axis(XboxGenericMsg& msg)
{
  switch (msg.type)
  {
    case XBOX_MSG_XBOX:
      squarify_axis(msg.xbox.x1, msg.xbox.y1);
      squarify_axis(msg.xbox.x2, msg.xbox.y2);
      break;

    case XBOX_MSG_XBOX360:
      squarify_axis(msg.xbox360.x1, msg.xbox360.y1);
      squarify_axis(msg.xbox360.x2, msg.xbox360.y2);
      break;
  }
}

static int16_t scale_deadzone(int16_t value, const int deadzone)
{
  float rv = value;
  if (value < -deadzone) {
    const float scale = 32768 / (32768 - deadzone);
    rv += deadzone;
    rv *= scale;
    rv -= 0.5;
  } else if (value > deadzone) {
    const float scale = 32767 / (32767 - deadzone);
    rv -= deadzone;
    rv *= scale;
    rv += 0.5;
  } else {
    return 0;
  }
  return clamp(-32768, static_cast<int>(rv), 32767);
}

static uint8_t scale_trigger_deadzone(uint8_t value, int deadzone)
{
  const float scale = 255 / (255 - deadzone);
    if (value <= deadzone) {
        return 0;
    } else {
        value -= deadzone;
        float rv = value * scale;
        return clamp(0, static_cast<int>(rv+0.5), 255);
    }
}

void apply_deadzone(XboxGenericMsg& msg, const Options& opts)
{
  switch (msg.type)
  {
    case XBOX_MSG_XBOX:
      msg.xbox.x1 = scale_deadzone(msg.xbox.x1, opts.deadzone);
      msg.xbox.y1 = scale_deadzone(msg.xbox.y1, opts.deadzone);
      msg.xbox.x2 = scale_deadzone(msg.xbox.x2, opts.deadzone);
      msg.xbox.y2 = scale_deadzone(msg.xbox.y2, opts.deadzone);
      msg.xbox.lt = scale_trigger_deadzone(msg.xbox.lt, opts.deadzone_trigger);
      msg.xbox.rt = scale_trigger_deadzone(msg.xbox.rt, opts.deadzone_trigger);
      break;

    case XBOX_MSG_XBOX360:
      msg.xbox360.x1 = scale_deadzone(msg.xbox360.x1, opts.deadzone);
      msg.xbox360.y1 = scale_deadzone(msg.xbox360.y1, opts.deadzone);
      msg.xbox360.x2 = scale_deadzone(msg.xbox360.x2, opts.deadzone);
      msg.xbox360.y2 = scale_deadzone(msg.xbox360.y2, opts.deadzone);
      msg.xbox360.lt = scale_trigger_deadzone(msg.xbox360.lt, opts.deadzone_trigger);
      msg.xbox360.rt = scale_trigger_deadzone(msg.xbox360.rt, opts.deadzone_trigger);
      break;
  }
}

void apply_axis_sensitivity(XboxGenericMsg& msg, const Options& opts)
{
  for(std::vector<AxisSensitivityMapping>::const_iterator i = opts.axis_sensitivity_map.begin();
      i != opts.axis_sensitivity_map.end(); ++i)
  {
    float pos = get_axis_float(msg, i->axis);
    float t = powf(2, i->sensitivity);

    if (pos > 0)
    {
      pos = powf(1.0f - powf(1.0f - pos, t), 1 / t);
      set_axis_float(msg, i->axis, pos);
    }
    else
    {
      pos = powf(1.0f - powf(1.0f - -pos, t), 1 / t);
      set_axis_float(msg, i->axis, -pos);
    }
  }
}

void apply_four_way_restrictor(XboxGenericMsg& msg, const Options& opts)
{
  // left Stick
  if (abs(get_axis(msg, XBOX_AXIS_X1)) > abs(get_axis(msg, XBOX_AXIS_Y1)))
  {
    set_axis(msg, XBOX_AXIS_Y1, 0);
  }
  else if (abs(get_axis(msg, XBOX_AXIS_Y1)) > abs(get_axis(msg, XBOX_AXIS_X1)))
  {
    set_axis(msg, XBOX_AXIS_X1, 0);
  }
  else
  {
    set_axis(msg, XBOX_AXIS_X1, 0);
  }

  // right stick
  if (abs(get_axis(msg, XBOX_AXIS_X2)) > abs(get_axis(msg, XBOX_AXIS_Y2)))
  {
    set_axis(msg, XBOX_AXIS_Y2, 0);
  }
  else if (abs(get_axis(msg, XBOX_AXIS_Y2)) > abs(get_axis(msg, XBOX_AXIS_X2)))
  {
    set_axis(msg, XBOX_AXIS_X2, 0);
  }
  else
  {
    set_axis(msg, XBOX_AXIS_X2, 0);
  }
}

void apply_dpad_rotator(XboxGenericMsg& msg, const Options& opts)
{
  int up    = get_button(msg, XBOX_DPAD_UP);
  int down  = get_button(msg, XBOX_DPAD_DOWN);
  int left  = get_button(msg, XBOX_DPAD_LEFT);
  int right = get_button(msg, XBOX_DPAD_RIGHT);

  // -1: not pressed, 0: up, 1: up/right, ...
  int direction = -1;
  if (up && !down && !left && !right)
  {
    direction = 0;
  }
  else if (up && !down && !left && right)
  {
    direction = 1;
  }
  else if (!up && !down && !left && right)
  {
    direction = 2;
  }
  else if (!up && down && !left && right)
  {
    direction = 3;
  }
  else if (!up && down && !left && !right)
  {
    direction = 4;
  }
  else if (!up && down && left && !right)
  {
    direction = 5;
  }
  else if (!up && !down && left && !right)
  {
    direction = 6;
  }
  else if (up && !down && left && !right)
  {
    direction = 7;
  }

  if (direction != -1)
  {  
    direction += opts.dpad_rotation;
    direction %= 8;
    if (direction < 0)
      direction += 8;

    // set everything to zero
    set_button(msg, XBOX_DPAD_UP,    0);
    set_button(msg, XBOX_DPAD_DOWN,  0);
    set_button(msg, XBOX_DPAD_LEFT,  0);
    set_button(msg, XBOX_DPAD_RIGHT, 0);

    // apply the given direction
    switch(direction)
    {
      case 0:
        set_button(msg, XBOX_DPAD_UP, 1);
        break;

      case 1:
        set_button(msg, XBOX_DPAD_UP, 1);
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        break;

      case 2:
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        break;

      case 3:
        set_button(msg, XBOX_DPAD_RIGHT, 1);
        set_button(msg, XBOX_DPAD_DOWN, 1);
        break;

      case 4:
        set_button(msg, XBOX_DPAD_DOWN, 1);
        break;

      case 5:
        set_button(msg, XBOX_DPAD_DOWN, 1);
        set_button(msg, XBOX_DPAD_LEFT, 1);
        break;

      case 6:
        set_button(msg, XBOX_DPAD_LEFT, 1);
        break;

      case 7:
        set_button(msg, XBOX_DPAD_UP, 1);
        set_button(msg, XBOX_DPAD_LEFT, 1);
        break;
    }
  }
}

/* EOF */
