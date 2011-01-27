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

#include "sensitivity_axis_filter.hpp"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <math.h>

#include "helper.hpp"

SensitivityAxisFilter*
SensitivityAxisFilter::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  
  float sensitivity = 0.0f;

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0: sensitivity = boost::lexical_cast<float>(*i); break;
      default: throw std::runtime_error("to many arguments");
    };
  }

  return new SensitivityAxisFilter(sensitivity);
}

SensitivityAxisFilter::SensitivityAxisFilter(float sensitivity) :
  m_sensitivity(sensitivity)
{  
}

int
SensitivityAxisFilter::filter(int value, int min, int max)
{
  float pos = to_float(value, min, max);

  float t = powf(2, m_sensitivity);

  // FIXME: there might be better/more standard ways to accomplish this
  if (pos > 0)
  {
    pos = powf(1.0f - powf(1.0f - pos, t), 1 / t);
    return from_float(pos, min, max);
  }
  else
  {
    pos = powf(1.0f - powf(1.0f - -pos, t), 1 / t);
    return from_float(-pos, min, max);
  }
}

std::string
SensitivityAxisFilter::str() const
{
  std::ostringstream out;
  out << "sensitivity:" << m_sensitivity;
  return out.str();
}

/* EOF */
