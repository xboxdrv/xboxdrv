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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_MACRO_BUTTON_EVENT_HANDLER_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_MACRO_BUTTON_EVENT_HANDLER_HPP

#include "button_event.hpp"
#include "ui_event.hpp"
#include "uinput.hpp"

class MacroButtonEventHandler : public ButtonEventHandler
{
public:
private:
  struct MacroEvent {
    enum { kInitOp, kSendOp, kWaitOp, kNull } type; 
    
    union {
      struct {
        UIEvent event;
        UIEventEmitterPtr* emitter;
	int minimum;
	int maximum;
	int fuzz;
	int flat;
      } init;

      struct {
        UIEvent event;
        UIEventEmitterPtr* emitter;
        int     value;
      } send;

      struct {
        int msec;
      } wait;
    };
  };

public:
  static MacroButtonEventHandler* from_file(UInput& uinput, int slot, bool extra_devices, const std::string& filename);
  static MacroButtonEventHandler* from_string(UInput& uinput, int slot, bool extra_devices, const std::string& str);

public:
  MacroButtonEventHandler(UInput& uinput, int slot, bool extra_devices,
                          const std::vector<MacroEvent>& events);
  ~MacroButtonEventHandler();

  void send(bool value);
  void update(int msec_delta);

  std::string str() const;

private:
  UIEventEmitterPtr get_emitter(UInput& uinput, const UIEvent& ev);

private:
  static MacroEvent macro_event_from_string(const std::string& str);

private:
  std::vector<MacroEvent> m_events;
  bool m_send_in_progress;
  int m_countdown;
  std::vector<MacroEvent>::size_type m_event_counter;

  typedef std::map<UIEvent, UIEventEmitterPtr> Emitter;
  Emitter m_emitter;
};

#endif

/* EOF */
