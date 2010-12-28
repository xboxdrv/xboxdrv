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

#ifndef HEADER_XBOXDRV_AXIS_FILTER_HPP
#define HEADER_XBOXDRV_AXIS_FILTER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

class AxisFilter;

typedef boost::shared_ptr<AxisFilter> AxisFilterPtr;

class AxisFilter
{
public:
  static AxisFilterPtr from_string(const std::string& str);

public:
  AxisFilter() {}
  virtual ~AxisFilter() {}

  virtual void update(int msec_delta) {}
  virtual int filter(int value, int min, int max) =0;
};

class InvertAxisFilter : public AxisFilter
{
public:
  InvertAxisFilter() {}
  ~InvertAxisFilter() {}

  int filter(int value, int min, int max);
};

class SensitivityAxisFilter : public AxisFilter
{
public:
  static SensitivityAxisFilter* from_string(const std::string& str);

public:
  SensitivityAxisFilter(float sensitivity);

  int filter(int value, int min, int max);

private:
  float m_sensitivity;
};

class CalibrationAxisFilter : public AxisFilter
{
public:
  static CalibrationAxisFilter* from_string(const std::string& str);

public:
  CalibrationAxisFilter(int min, int center, int max);

  int filter(int value, int min, int max);

private:
  int m_min;
  int m_center;
  int m_max;
};

class DeadzoneAxisFilter : public AxisFilter
{
public:
  static DeadzoneAxisFilter* from_string(const std::string& str);

public:
  DeadzoneAxisFilter(int deadzone, bool smooth);

  int filter(int value, int min, int max);

private:
  int m_deadzone;
  bool m_smooth;
};

class RelativeAxisFilter : public AxisFilter
{
public:
  static RelativeAxisFilter* from_string(const std::string& str);

public:
  RelativeAxisFilter(int speed);

  void update(int msec_delta);
  int filter(int value, int min, int max);

private:
  int m_speed;

  int m_value;
  int m_state;
  int m_min;
  int m_max;
};

#endif

/* EOF */
