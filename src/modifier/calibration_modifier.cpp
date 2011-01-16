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

CalibrationMapping CalibrationMapping::from_string(const std::string& lhs, const std::string& rhs)
{
  CalibrationMapping mapping; 
  mapping.m_axis    = string2axis(lhs);
  mapping.m_min     = -32768;
  mapping.m_center  = 0;
  mapping.m_max     = 32767;

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
          mapping.m_min = boost::lexical_cast<int>(*i);
        else if (j == 1)
          mapping.m_center = boost::lexical_cast<int>(*i);
        else if (j == 2)
          mapping.m_max = boost::lexical_cast<int>(*i);
        else 
          throw std::runtime_error("--calibration: to many arguments given, syntax is 'AXIS=MIN:CENTER:MAX'");
      }
      catch(boost::bad_lexical_cast&) 
      {
        throw std::runtime_error("--calibration: couldn't convert '" + *i + "' to int");
      }
    }
  }
      
  if (!(mapping.m_min <= mapping.m_center && mapping.m_center <= mapping.m_max))
    throw std::runtime_error("Order wrong 'AXIS=MIN:CENTER:MAX'");

  return mapping;
}

CalibrationMapping::CalibrationMapping() :
  m_axis(XBOX_AXIS_UNKNOWN),
  m_min(0),
  m_center(0),
  m_max(0)
{  
}

void
CalibrationMapping::update(int msec_delta, XboxGenericMsg& msg)
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

CalibrationModifier::CalibrationModifier(const std::vector<CalibrationMapping>& calibration_map) :
  m_calibration_map(calibration_map)
{
}

void
CalibrationModifier::update(int msec_delta, XboxGenericMsg& msg)
{
  for(std::vector<CalibrationMapping>::const_iterator i = m_calibration_map.begin(); i != m_calibration_map.end(); ++i)
  {
    int value = get_axis(msg,  i->m_axis);

    if (value < i->m_center)
      value = 32768 * (value - i->m_center) / (i->m_center - i->m_min);
    else if (value > i->m_center)
      value = 32767 * (value - i->m_center) / (i->m_max - i->m_center);
    else
      value = 0;

    set_axis(msg, i->m_axis, clamp(-32768, 32767, value));
  }
}

/* EOF */
