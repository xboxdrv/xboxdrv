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
#include <stdexcept>

#include "axisfilter/calibration_axis_filter.hpp"
#include "axisfilter/const_axis_filter.hpp"
#include "axisfilter/deadzone_axis_filter.hpp"
#include "axisfilter/invert_axis_filter.hpp"
#include "axisfilter/log_axis_filter.hpp"
#include "axisfilter/lowpass_axis_filter.hpp"
#include "axisfilter/relative_axis_filter.hpp"
#include "axisfilter/response_curve_axis_filter.hpp"
#include "axisfilter/sensitivity_axis_filter.hpp"

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
  else if (filtername == "const")
  {
    return AxisFilterPtr(ConstAxisFilter::from_string(rest));
  }
  else if (filtername == "relative" || filtername == "rel")
  {
    return AxisFilterPtr(RelativeAxisFilter::from_string(rest));
  }
  else if (filtername == "resp" || filtername == "response" || filtername == "responsecurve")
  {
    return AxisFilterPtr(ResponseCurveAxisFilter::from_string(rest));
  }
  else if (filtername == "lowpass")
  {
    return LowpassAxisFilter::from_string(rest);
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

/* EOF */
