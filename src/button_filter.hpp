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

#ifndef HEADER_XBOXDRV_BUTTON_FILTER_HPP
#define HEADER_XBOXDRV_BUTTON_FILTER_HPP

#include <boost/shared_ptr.hpp>
#include <string>

class ButtonFilter;

typedef boost::shared_ptr<ButtonFilter> ButtonFilterPtr;

class ButtonFilter
{
public:
  static ButtonFilterPtr from_string(const std::string& str);

public:
  ButtonFilter() {}
  virtual ~ButtonFilter() {}

  virtual bool filter(bool value) =0;
  virtual void update(int msec_delta) {}
  virtual std::string str() const = 0;
};

class ToggleButtonFilter : public ButtonFilter
{
public:
  ToggleButtonFilter();

  bool filter(bool value);
  void update(int msec_delta) {}
  std::string str() const;

private:
  bool m_state;
  bool m_last_value;
};

class InvertButtonFilter : public ButtonFilter
{
public:
  InvertButtonFilter() {}

  void update(int msec_delta) {}
  bool filter(bool value);
  std::string str() const;
};

class AutofireButtonFilter : public ButtonFilter
{
public:
  static AutofireButtonFilter* from_string(const std::string& str);

public:
  AutofireButtonFilter(int rate, int delay);

  void update(int msec_delta);
  bool filter(bool value);
  std::string str() const;

private:
  bool m_state;
  bool m_autofire;

  /** msec between shots */
  int m_rate;
  int m_delay;
  int m_counter;
};

class LogButtonFilter : public ButtonFilter
{
public:
  static LogButtonFilter* from_string(const std::string& str);

public:
  LogButtonFilter(const std::string& name);

  bool filter(bool value);
  std::string str() const;

private:
  std::string m_name;
};

#endif

/* EOF */
