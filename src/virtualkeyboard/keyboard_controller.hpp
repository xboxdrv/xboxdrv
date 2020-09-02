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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_CONTROLLER_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_CONTROLLER_HPP

#include <string>
#include <linux/input.h>
#include <glib.h>

#include "uinput/ui_event_emitter.hpp"

class VirtualKeyboard;
class UInput;

class KeyboardController
{
private:
  VirtualKeyboard& m_keyboard;
  UInput& m_uinput;
  std::string m_device;
  int m_fd;
  GIOChannel* m_io_channel;
  int m_timeout_source;

  float m_stick_x;
  float m_stick_y;

  float m_stick2_x;
  float m_stick2_y;

  int m_timer_x;
  int m_timer_y;

  enum {
    kSendButton = BTN_A,
    kHoldButton = BTN_Y,
    kBackspaceButton  = BTN_X,
    kCancelHoldButton = BTN_B,
    kShiftButton = BTN_TL,
    kCtrlButton  = BTN_TR,
    kHideButton  = BTN_MODE
  };

  UIEventEmitterPtr m_backspace_key;
  UIEventEmitterPtr m_shift_key;
  UIEventEmitterPtr m_ctrl_key;

public:
  KeyboardController(VirtualKeyboard& keyboard, UInput& uinput, const std::string& device);
  ~KeyboardController();

  void parse(const struct input_event& ev);
  gboolean on_read_data(GIOChannel* source, GIOCondition condition);
  bool on_timeout();

private:
  void sync();

private:
  static gboolean on_read_data_wrap(GIOChannel* source, GIOCondition condition, gpointer userdata) {
    return static_cast<KeyboardController*>(userdata)->on_read_data(source, condition);
  }

  static gboolean on_timeout_wrap(gpointer data) {
    return static_cast<KeyboardController*>(data)->on_timeout();
  }

private:
  KeyboardController(const KeyboardController&);
  KeyboardController& operator=(const KeyboardController&);
};

#endif

/* EOF */
