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

#ifndef HEADER_XBOXDRV_AXIS_EVENT_HPP
#define HEADER_XBOXDRV_AXIS_EVENT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <vector>

#include "axis_filter.hpp"
#include "uinput_deviceid.hpp"

class uInput;
class AxisEvent;
class AxisEventHandler;

typedef boost::shared_ptr<AxisEvent> AxisEventPtr;

class AxisEvent
{
public:
  static AxisEventPtr invalid();

  static AxisEventPtr create_abs(int device_id, int code, int min, int max, int fuzz, int flat);
  static AxisEventPtr create_rel(int device_id, int code, int repeat = 10, float value = 5);

  /** If an AxisEvent gets created the user has to set min/max with set_axis_range() */ 
  static AxisEventPtr from_string(const std::string& str);

public:
  AxisEvent(AxisEventHandler* handler, int min = 0, int max = 0);
  ~AxisEvent() {}

  void add_filter(AxisFilterPtr filter);

  void init(uInput& uinput) const;
  void send(uInput& uinput, int value);
  void update(uInput& uinput, int msec_delta);

  void set_axis_range(int min, int max);

  std::string str() const;

private:
  int  m_last_raw_value;
  int  m_last_send_value;
  int  m_min;
  int  m_max;
  boost::scoped_ptr<AxisEventHandler> m_handler;
  std::vector<AxisFilterPtr> m_filters;
};

class AxisEventHandler
{
public:
  virtual ~AxisEventHandler() {}

  virtual void init(uInput& uinput) const =0;
  virtual void send(uInput& uinput, int value) =0;
  virtual void update(uInput& uinput, int msec_delta) =0;

  virtual void set_axis_range(int min, int max) {}

  virtual std::string str() const =0;
};

class RelAxisEventHandler : public AxisEventHandler
{
public:
  static RelAxisEventHandler* from_string(const std::string& str);

public:
  RelAxisEventHandler();
  RelAxisEventHandler(int device_id, int code, int repeat = 10, float value = 5);

  void init(uInput& uinput) const;
  void send(uInput& uinput, int value);
  void update(uInput& uinput, int msec_delta);

  std::string str() const;

private:
  UIEvent m_code;
  float m_value; // FIXME: Why is this float?
  int   m_repeat;
};

class AbsAxisEventHandler : public AxisEventHandler
{
public:
  static AbsAxisEventHandler* from_string(const std::string& str);

public:
  AbsAxisEventHandler();
  AbsAxisEventHandler(const UIEvent& code, int min, int max, int fuzz, int flat);

  void set_axis_range(int min, int max);

  void init(uInput& uinput) const;
  void send(uInput& uinput, int value);
  void update(uInput& uinput, int msec_delta);

  std::string str() const;

private:
  UIEvent m_code;
  int m_min;
  int m_max;
  int m_fuzz;
  int m_flat;
};

class KeyAxisEventHandler : public AxisEventHandler
{
public:
  static KeyAxisEventHandler* from_string(const std::string& str);
  
public:
  KeyAxisEventHandler();

  void init(uInput& uinput) const;
  void send(uInput& uinput, int value);
  void update(uInput& uinput, int msec_delta);

  std::string str() const;

private:
  static const int MAX_MODIFIER = 4;

  int m_old_value;

  // Array is terminated by -1
  UIEvent m_up_codes[MAX_MODIFIER+1];
  UIEvent m_down_codes[MAX_MODIFIER+1];
  int m_threshold;
};

#endif

/* EOF */
