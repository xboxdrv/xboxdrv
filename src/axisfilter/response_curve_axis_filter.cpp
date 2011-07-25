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

#include "response_curve_axis_filter.hpp"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

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
    float bucket_size  = static_cast<float>(max - min) / static_cast<float>(bucket_count);
      
    int bucket_index = static_cast<int>(static_cast<float>(value - min) / bucket_size);

    float t = (static_cast<float>(value - min) - (static_cast<float>(bucket_index) * bucket_size)) / bucket_size;
      
    return static_cast<int>(((1.0f - t) * static_cast<float>(m_samples[bucket_index])) + 
                            (t * static_cast<float>(m_samples[bucket_index + 1])));
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

/* EOF */
