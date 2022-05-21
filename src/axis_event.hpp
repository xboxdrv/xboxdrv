/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include <memory>

#include <uinpp/ui_event.hpp>

#include "axis_filter.hpp"

class AxisEvent;
class AxisEventHandler;

typedef std::shared_ptr<AxisEvent> AxisEventPtr;

class AxisEvent
{
public:
  AxisEvent(AxisEventHandler* handler);
  ~AxisEvent() {}

  void add_filter(AxisFilterPtr filter);

  void send(int value, int min, int max);
  void update(int msec_delta);

  std::string str() const;

private:
  int  m_last_raw_value;
  int  m_last_send_value;
  int  m_last_min;
  int  m_last_max;
  std::unique_ptr<AxisEventHandler> m_handler;
  std::vector<AxisFilterPtr> m_filters;
};

class AxisEventHandler
{
public:
  AxisEventHandler();
  virtual ~AxisEventHandler() {}

  virtual void send(int value, int min, int max) =0;
  virtual void update(int msec_delta) =0;

  virtual std::string str() const =0;
};

#endif

/* EOF */
