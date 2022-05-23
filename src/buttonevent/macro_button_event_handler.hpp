/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
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

#include <uinpp/event.hpp>
#include <uinpp/multi_device.hpp>

#include "button_event.hpp"

class MacroButtonEventHandler : public ButtonEventHandler
{
public:
private:
  struct MacroEvent {
    enum { kInitOp, kSendOp, kWaitOp, kNull } type;

    struct InitEvent {
      uinpp::Event event;
      uinpp::EventEmitter* emitter;
      int minimum;
      int maximum;
      int fuzz;
      int flat;
    };

    struct SendEvent {
      uinpp::Event event;
      uinpp::EventEmitter* emitter;
      int     value;
    };

    struct WaitEvent {
      int msec;
    };

    union {
      InitEvent init;
      SendEvent send;
      WaitEvent wait;
    };
  };

public:
  static MacroButtonEventHandler* from_file(uinpp::MultiDevice& uinput, int slot, bool extra_devices, const std::string& filename);
  static MacroButtonEventHandler* from_string(uinpp::MultiDevice& uinput, int slot, bool extra_devices, const std::string& str);

public:
  MacroButtonEventHandler(uinpp::MultiDevice& uinput, int slot, bool extra_devices,
                          const std::vector<MacroEvent>& events);
  ~MacroButtonEventHandler();

  void send(bool value) override;
  void update(int msec_delta) override;

  std::string str() const override;

private:
  uinpp::EventEmitter* get_emitter(uinpp::MultiDevice& uinput, const uinpp::Event& ev);

private:
  static MacroEvent macro_event_from_string(const std::string& str);

private:
  std::vector<MacroEvent> m_events;
  bool m_send_in_progress;
  int m_countdown;
  std::vector<MacroEvent>::size_type m_event_counter;

  typedef std::map<uinpp::Event, uinpp::EventEmitter*> Emitter;
  Emitter m_emitter;
};

#endif

/* EOF */
