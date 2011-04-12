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

#ifndef HEADER_XBOXDRV_BUTTON_EVENT_HPP
#define HEADER_XBOXDRV_BUTTON_EVENT_HPP

#include <boost/scoped_ptr.hpp>
#include <vector>

#include "button_filter.hpp"
#include "ui_event.hpp"

class UInput;
class ButtonEvent;
class ButtonEventHandler;

typedef boost::shared_ptr<ButtonEvent> ButtonEventPtr;

class ButtonEvent
{
public:
  static ButtonEventPtr invalid();
  static ButtonEventPtr create(ButtonEventHandler* handler);
  static ButtonEventPtr create_key(int code);
  static ButtonEventPtr create_key(int device_id, int code);
  static ButtonEventPtr create_key();
  static ButtonEventPtr create_abs(int code);
  static ButtonEventPtr create_rel(int code);
  static ButtonEventPtr from_string(const std::string& str, const std::string& directory);

protected:
  ButtonEvent(ButtonEventHandler* handler);

public: 
  void init(UInput& uinput, int slot, bool extra_devices);
  void send(UInput& uinput, bool value);
  void update(UInput& uinput, int msec_delta);
  std::string str() const;

  void add_filters(const std::vector<ButtonFilterPtr>& filters);
  void add_filter(ButtonFilterPtr filter);

private:
  bool m_last_send_state;
  bool m_last_raw_state;
  boost::scoped_ptr<ButtonEventHandler> m_handler;
  std::vector<ButtonFilterPtr> m_filters;
};

class ButtonEventHandler
{
public:
  virtual ~ButtonEventHandler() {}
  
  virtual void init(UInput& uinput, int slot, bool extra_devices) =0;
  virtual void send(UInput& uinput, bool value) =0;
  virtual void update(UInput& uinput, int msec_delta) =0;
  virtual std::string str() const =0;
};

#endif

/* EOF */
