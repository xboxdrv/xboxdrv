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

#include "calibration_modifier.hpp"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace {

int clamp(int lhs, int rhs, int v)
{
  return std::max(lhs, std::min(v, rhs));
}

} // namespace

CalibrationModifier*
CalibrationModifier::from_string(const std::vector<std::string>& args)
{
  if (args.size() != 3)
  {
    throw std::runtime_error("CalibrationModifier takes exactly three arguments");
  }
  else
  {
    return new CalibrationModifier(string2axis(args[0]),
                                   boost::lexical_cast<int>(args[0]),
                                   boost::lexical_cast<int>(args[1]),
                                   boost::lexical_cast<int>(args[2]));
  }
}

CalibrationModifier*
CalibrationModifier::from_string(const std::string& lhs, const std::string& rhs)
{
  std::auto_ptr<CalibrationModifier> mapping(new CalibrationModifier);

  mapping->m_axis    = string2axis(lhs);
  mapping->m_min     = -32768;
  mapping->m_center  = 0;
  mapping->m_max     = 32767;

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
          mapping->m_min = boost::lexical_cast<int>(*i);
        else if (j == 1)
          mapping->m_center = boost::lexical_cast<int>(*i);
        else if (j == 2)
          mapping->m_max = boost::lexical_cast<int>(*i);
        else 
          throw std::runtime_error("--calibration: to many arguments given, syntax is 'AXIS=MIN:CENTER:MAX'");
      }
      catch(boost::bad_lexical_cast&) 
      {
        throw std::runtime_error("--calibration: couldn't convert '" + *i + "' to int");
      }
    }
  }
      
  if (!(mapping->m_min <= mapping->m_center && mapping->m_center <= mapping->m_max))
    throw std::runtime_error("Order wrong 'AXIS=MIN:CENTER:MAX'");

  return mapping.release();
}

CalibrationModifier::CalibrationModifier(XboxAxis axis, int min, int center, int max) :
  m_axis(axis),
  m_min(min),
  m_center(center),
  m_max(max)
{
}

CalibrationModifier::CalibrationModifier() :
  m_axis(XBOX_AXIS_UNKNOWN),
  m_min(0),
  m_center(0),
  m_max(0)
{  
}

void
CalibrationModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  int value = get_axis(msg,  m_axis);

  if (value < m_center)
    value = 32768 * (value - m_center) / (m_center - m_min);
  else if (value > m_center)
    value = 32767 * (value - m_center) / (m_max - m_center);
  else
    value = 0;

  set_axis(msg, m_axis, clamp(-32768, 32767, value));
}

/* EOF */
