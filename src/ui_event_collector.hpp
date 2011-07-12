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

#ifndef HEADER_XBOXDRV_UI_EVENT_COLLECTOR_HPP
#define HEADER_XBOXDRV_UI_EVENT_COLLECTOR_HPP

#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <vector>

#include "ui_event_emitter.hpp"

class UIEventCollector;
class UInput;

typedef boost::shared_ptr<UIEventCollector> UIEventCollectorPtr;

class UIEventCollector
{
private:
  UInput& m_uinput;
  uint32_t m_device_id;
  int m_type;
  int m_code;

  int m_value;

  typedef std::vector<UIEventEmitterPtr> Emitters;
  Emitters m_emitters;
  
public:
  UIEventCollector(UInput& uinput, uint32_t device_id, int type, int code);

  uint32_t get_device_id() const { return m_device_id; }
  int      get_type() const { return m_type; }
  int      get_code() const { return m_code; }

  UIEventEmitterPtr create_emitter();

  void sync();

private:
  UIEventCollector(const UIEventCollector&);
  UIEventCollector& operator=(const UIEventCollector&);
};

#endif

/* EOF */
