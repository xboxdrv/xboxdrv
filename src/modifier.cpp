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

ButtonMapping 
ButtonMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  ButtonMapping mapping;

  mapping.lhs = XBOX_BTN_UNKNOWN;
  mapping.rhs = XBOX_BTN_UNKNOWN;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.lhs = string2btn(*t); break;
      default: mapping.filters.push_back(ButtonFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.rhs = mapping.lhs;
  }
  else
  {
    mapping.rhs = string2btn(rhs);
  }

  return mapping;
}

AxisMapping
AxisMapping::from_string(const std::string& lhs_, const std::string& rhs)
{
  std::string lhs = lhs_;
  AxisMapping mapping;

  mapping.invert = false;
  mapping.lhs = XBOX_AXIS_UNKNOWN;
  mapping.rhs = XBOX_AXIS_UNKNOWN;

  if (lhs[0] == '-')
  {
    mapping.invert = true;
    lhs = lhs.substr(1);
  }

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(lhs, boost::char_separator<char>("^", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:  mapping.lhs = string2axis(*t); break;
      default: mapping.filters.push_back(AxisFilter::from_string(*t));
    }
  }

  if (rhs.empty())
  {
    mapping.rhs = mapping.lhs;
  }
  else
  {
    mapping.rhs = string2axis(rhs);
  }

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

/* EOF */
