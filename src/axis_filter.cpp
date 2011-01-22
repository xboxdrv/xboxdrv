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

#include "axis_filter.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <math.h>

#include "helper.hpp"

namespace {

/** converts the arbitary range to [-1,1] */
inline float to_float(int value, int min, int max)
{
  return static_cast<float>(value - min) / static_cast<float>(max - min) * 2.0f - 1.0f;
}

/** converts the range [-1,1] to [min,max] */
inline int from_float(float value, int min, int max)
{
  return (value + 1.0f) / 2.0f * static_cast<float>(max - min) + min;
}

} // namespace

AxisFilterPtr
AxisFilter::from_string(const std::string& str)
{
  std::string::size_type p = str.find(':');
  const std::string& filtername = str.substr(0, p);
  std::string rest;

  if (p != std::string::npos) 
    rest = str.substr(p+1);

  if (filtername == "invert" || filtername == "inv")
  {
    return AxisFilterPtr(new InvertAxisFilter);
  }
  else if (filtername == "calibration" || filtername == "cal")
  {
    return AxisFilterPtr(CalibrationAxisFilter::from_string(rest));
  }
  else if (filtername == "sensitivity" || filtername == "sen")
  {
    return AxisFilterPtr(SensitivityAxisFilter::from_string(rest));
  }
  else if (filtername == "deadzone" || filtername == "dead")
  {
    return AxisFilterPtr(DeadzoneAxisFilter::from_string(rest));
  }
  else if (filtername == "relative" || filtername == "rel")
  {
    return AxisFilterPtr(RelativeAxisFilter::from_string(rest));
  }
  else if (filtername == "resp" || filtername == "response" || filtername == "responsecurve")
  {
    return AxisFilterPtr(ResponseCurveAxisFilter::from_string(rest));
  }
  else if (filtername == "log")
  {
    return AxisFilterPtr(LogAxisFilter::from_string(rest));
  }
  else
  {
    std::ostringstream out;
    out << "unknown AxisFilter '" << filtername << "'";
    throw std::runtime_error(out.str());
  }
}

int
InvertAxisFilter::filter(int value, int min, int max)
{
  int center = (max + min + 1)/2; // FIXME: '+1' is kind of a hack to
                                  // get the center at 0 for the
                                  // [-32768, 32767] case
  if (value < center)
  {
    return (max - center) * (value - center) / (min - center) + center;
  }
  else if (value > center)
  {
    return (min - center) * (value - center) / (max - center) + center;
  }
  else
  {
    return value;
  }
}

std::string
InvertAxisFilter::str() const
{
  std::ostringstream out;
  out << "invert";
  return out.str();
}

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

CalibrationAxisFilter*
CalibrationAxisFilter::from_string(const std::string& str)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));

  int min    = 0;
  int center = 0;
  int max    = 0;

  int j = 0;
  for(tokenizer::iterator i = tokens.begin(); i != tokens.end(); ++i, ++j)
  {
    switch(j)
    {
      case 0: min    = boost::lexical_cast<int>(*i); break;
      case 1: center = boost::lexical_cast<int>(*i); break;
      case 2: max    = boost::lexical_cast<int>(*i); break;
      default: throw std::runtime_error("to many arguments");
    };
  }

  return new CalibrationAxisFilter(min, center, max);
}

CalibrationAxisFilter::CalibrationAxisFilter(int min, int center, int max) :
  m_min(min),
  m_center(center),
  m_max(max)
{
}

int
CalibrationAxisFilter::filter(int value, int min, int max)
{
  if (value < m_center)
    value = -min * (value - m_center) / (m_center - m_min);
  else if (value > m_center)
    value = max * (value - m_center) / (m_max - m_center);
  else
    value = 0;

  return Math::clamp(min, value, max);
}

std::string
CalibrationAxisFilter::str() const
{
  std::ostringstream out;
  out << "calibration:" << m_min << ":" << m_center << ":" << m_max;
  return out.str();
}

DeadzoneAxisFilter*
DeadzoneAxisFilter::from_string(const std::string& str)
{
  int  min_deadzone = 0;
  int  max_deadzone = 0;
  bool smooth   = true;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0:
        min_deadzone = -boost::lexical_cast<int>(*t);
        max_deadzone = -min_deadzone;
        break;
        
      case 1:
        max_deadzone = boost::lexical_cast<int>(*t); 
        break;

      case 2:
        smooth = boost::lexical_cast<bool>(*t); 
        break;

      default:
        throw std::runtime_error("to many arguments"); 
        break;
    }
  }

  return new DeadzoneAxisFilter(min_deadzone, max_deadzone, smooth);
}

DeadzoneAxisFilter::DeadzoneAxisFilter(int min_deadzone, int max_deadzone, bool smooth) :
  m_min_deadzone(min_deadzone),
  m_max_deadzone(max_deadzone),
  m_smooth(smooth)
{
}

int
DeadzoneAxisFilter::filter(int value, int min, int max)
{
  if (!m_smooth)
  {
    if (value < m_min_deadzone || m_max_deadzone < value)
    {
      return value;
    }
    else
    {
      return 0;
    }
  }
  else // (m_smooth)
  {
    if (value < m_min_deadzone)
    {
      return min * (value - m_min_deadzone) / (min - m_min_deadzone);
    }
    else if (value > m_max_deadzone) 
    {
      return max * (value - m_max_deadzone) / (max - m_max_deadzone);
    }
    else 
    {
      return 0;
    }
  }
}

std::string
DeadzoneAxisFilter::str() const
{
  std::ostringstream out;
  out << "deadzone:" << m_min_deadzone << ":" << m_max_deadzone << ":" << m_smooth;
  return out.str();
}

RelativeAxisFilter*
RelativeAxisFilter::from_string(const std::string& str)
{
  int speed = 20000;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    switch(idx)
    {
      case 0: speed = boost::lexical_cast<int>(*t); break;
      default: throw std::runtime_error("to many arguments"); break;
    }
  }

  return new RelativeAxisFilter(speed);
}

RelativeAxisFilter::RelativeAxisFilter(int speed) :
  m_speed(speed),
  m_float_speed(0.0f),
  m_value(0),
  m_state(0)
{
}

void
RelativeAxisFilter::update(int msec_delta)
{
  m_state += m_float_speed * m_value * msec_delta / 1000.0f;
  m_state = Math::clamp(-1.0f, m_state, 1.0f);
}

int
RelativeAxisFilter::filter(int value, int min, int max)
{
  m_value = to_float(value, min, max);

  m_float_speed = to_float(m_speed, min, max);

  return from_float(m_state, min, max);
}

std::string
RelativeAxisFilter::str() const
{
  std::ostringstream out;
  out << "relativeaxis:" << m_speed;
  return out.str();
}

ResponseCurveAxisFilter*
ResponseCurveAxisFilter::from_string(const std::string& str)
{
  std::vector<int> samples;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(str, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  int idx = 0;
  for(tokenizer::iterator t = tokens.begin(); t != tokens.end(); ++t, ++idx)
  {
    samples.push_back(boost::lexical_cast<int>(*t));
  }

  return new ResponseCurveAxisFilter(samples);
}

ResponseCurveAxisFilter::ResponseCurveAxisFilter(const std::vector<int>& samples) :
  m_samples(samples)
{
}

int
ResponseCurveAxisFilter::filter(int value, int min, int max)
{
  if (m_samples.empty())
  {
    return value;
  }
  else if (m_samples.size() == 1)
  {
    return m_samples[0];
  }
  else
  {
    // FIXME: should rewrite this to use integer only and make sure
    // that the edge conditions are meet
    int   bucket_count = m_samples.size() - 1;
    float bucket_size  = (max - min) / static_cast<float>(bucket_count);
      
    int bucket_index = int((value - min) / bucket_size);

    float t = ((value - min) - (static_cast<float>(bucket_index) * bucket_size)) / bucket_size;
      
    return ((1.0f - t) * m_samples[bucket_index]) + (t * m_samples[bucket_index + 1]);
  }
}

std::string
ResponseCurveAxisFilter::str() const
{
  std::ostringstream out;
  out << "responsecurve";
  for(std::vector<int>::const_iterator i = m_samples.begin(); i != m_samples.end(); ++i)
  {
    out << ":" << *i;
  }
  return out.str();
}

LogAxisFilter*
LogAxisFilter::from_string(const std::string& str)
{
  return new LogAxisFilter(str);
}

LogAxisFilter::LogAxisFilter(const std::string& name) :
  m_name(name)
{
}

int
LogAxisFilter::filter(int value, int min, int max)
{
  if (m_name.empty())
  {
    std::cout << value << std::endl;
  }
  else
  {
    std::cout << m_name << ": " << value << std::endl;    
  }

  return value;
}

std::string
LogAxisFilter::str() const
{
  std::ostringstream out;
  out << "log:" << m_name;
  return out.str();
}

/* EOF */
