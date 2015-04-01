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

#include "virtualkeyboard/keyboard_controller.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdexcept>
#include <math.h>

#include "log.hpp"
#include "uinput.hpp"
#include "virtualkeyboard/virtual_keyboard.hpp"

KeyboardController::KeyboardController(VirtualKeyboard& keyboard, UInput& uinput,
                                       const std::string& device) :
  m_keyboard(keyboard),
  m_uinput(uinput),
  m_device(device),
  m_fd(-1),
  m_io_channel(0),
  m_timeout_source(-1),
  m_stick_x(0),
  m_stick_y(0),
  m_stick2_x(0),
  m_stick2_y(0),
  m_timer_x(0),
  m_timer_y(0),
  m_backspace_key(),
 m_shift_key(),
 m_ctrl_key()
{
  m_fd = open(m_device.c_str(), O_RDONLY | O_NONBLOCK);

  if (m_fd == -1)
  {
    throw std::runtime_error(m_device + ": " + std::string(strerror(errno)));
  }

  m_io_channel = g_io_channel_unix_new(m_fd);

  // set encoding to binary
  GError* error = NULL;
  if (g_io_channel_set_encoding(m_io_channel, NULL, &error) != G_IO_STATUS_NORMAL)
  {
    log_error(error->message);
    g_error_free(error);
  }
  g_io_channel_set_buffered(m_io_channel, false);

  g_io_add_watch(m_io_channel,
                 static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                 &KeyboardController::on_read_data_wrap, this);

  m_backspace_key = m_uinput.add_key(0, KEY_BACKSPACE);
  m_shift_key     = m_uinput.add_key(0, KEY_LEFTSHIFT);
  m_ctrl_key      = m_uinput.add_key(0, KEY_LEFTCTRL);
}

KeyboardController::~KeyboardController()
{
  g_io_channel_unref(m_io_channel);
  close(m_fd);

  if (m_timeout_source > 0)
    g_source_remove(m_timeout_source);
}

void
KeyboardController::parse(const struct input_event& ev)
{
  //log_tmp(ev.type << " " << ev.code << " " << ev.value);
  if (ev.type == EV_ABS)
  {
    if (ev.code == ABS_HAT0X)
    {
      if (ev.value == -1)
      {
        m_keyboard.cursor_left();
      }
      else if (ev.value == 1)
      {
        m_keyboard.cursor_right();
      }
    }
    else if (ev.code == ABS_HAT0Y)
    {
      if (ev.value == -1)
      {
        m_keyboard.cursor_up();
      }
      else if (ev.value == 1)
      {
        m_keyboard.cursor_down();
      }
    }
    else if (ev.code == ABS_X)
    {
      if (abs(ev.value) > 8000)
      {
        //m_keyboard.move(ev.value / 4, 0);
        m_stick_x = static_cast<float>(ev.value) / 32768.0f;
      }
      else
      {
        m_stick_x = 0.0f;
      }
    }
    else if (ev.code == ABS_Y)
    {
      if (abs(ev.value) > 8000)
      {
        //m_keyboard.move(ev.value / 4, 0);
        m_stick_y = static_cast<float>(ev.value) / 32768.0f;
      }
      else
      {
        m_stick_y = 0.0f;
      }
    }
    else if (ev.code == ABS_RX)
    {
      if (abs(ev.value) > 6000)
      {
        //m_keyboard.move(ev.value / 4, 0);
        m_stick2_x = static_cast<float>(ev.value) / 32768.0f;
      }
      else
      {
        m_stick2_x = 0.0f;
      }
    }
    else if (ev.code == ABS_RY)
    {
      if (abs(ev.value) > 6000)
      {
        m_stick2_y = static_cast<float>(ev.value) / 32768.0f;
        //m_keyboard.move(0, ev.value / 40);
      }
      else
      {
        m_stick2_y = 0.0f;
      }
    }
  }
  else if (ev.type == EV_KEY)
  {
    switch(ev.code)
    {
      case kSendButton:
        m_keyboard.send_key(ev.value);
        break;

      case kHoldButton:
        //m_hold_key.send_key();
        break;

      case kCancelHoldButton:
        //m_keyboard.cancel_holds();
        break;

      case kShiftButton:
        log_tmp("Shift: " << ev.value);
        m_shift_key->send(ev.value);
        m_keyboard.set_shift_mode(ev.value);
        break;

      case kCtrlButton:
        log_tmp("Ctrl: " << ev.value);
        m_ctrl_key->send(ev.value);
        break;

      case kBackspaceButton:
        log_tmp("Backspace: " << ev.value);
        m_backspace_key->send(ev.value);
        break;

      case kHideButton:
        if (ev.value)
        {
          m_keyboard.show();
        }
        else
        {
          m_keyboard.hide();
        }
        break;
    }
  }
}

void
KeyboardController::sync()
{
  if (m_timeout_source > 0)
  {
    if (m_stick_x  == 0 && m_stick_y  == 0 &&
        m_stick2_x == 0 && m_stick2_y == 0)
    {
      g_source_remove(m_timeout_source);
      m_timeout_source = -1;
    }
  }
  else
  {
    if (m_stick_x  != 0 || m_stick_y  != 0 ||
        m_stick2_x != 0 || m_stick2_y != 0)
    {
      m_timeout_source = g_timeout_add(25, &KeyboardController::on_timeout_wrap, this);
    }
  }

  m_uinput.sync();
}

gboolean
KeyboardController::on_read_data(GIOChannel* source, GIOCondition condition)
{
  // read data
  struct input_event ev[128];
  int rd = 0;
  while((rd = ::read(m_fd, ev, sizeof(struct input_event) * 128)) > 0)
  {
    for (size_t i = 0; i < rd / sizeof(struct input_event); ++i)
    {
      parse(ev[i]);
    }
  }

  sync();

  return TRUE;
}

bool
KeyboardController::on_timeout()
{
  { // left stick
    const int m_repeat = 100;

    m_timer_x += static_cast<int>(static_cast<float>(25) * fabsf(m_stick_x));
    m_timer_y += static_cast<int>(static_cast<float>(25) * fabsf(m_stick_y));

    // FIXME: should reset m_timer when direction changes
    while(m_timer_x > m_repeat)
    {
      if (m_stick_x < 0)
      {
        m_keyboard.cursor_left();
      }
      else
      {
        m_keyboard.cursor_right();
      }

      m_timer_x -= m_repeat;
    }

    while(m_timer_y > m_repeat)
    {
      if (m_stick_y < 0)
      {
        m_keyboard.cursor_up();
      }
      else
      {
        m_keyboard.cursor_down();
      }

      m_timer_y -= m_repeat;
    }
  }

  { // right stick
    int x;
    int y;

    m_keyboard.get_position(&x, &y);

    // FIXME: should take actual time, as on_timeout() might not be reliable
    x += static_cast<int>(static_cast<float>(m_stick2_x) * 40.0f);
    y += static_cast<int>(static_cast<float>(m_stick2_y) * 40.0f);

    m_keyboard.move(x, y);
  }

  return true;
}

/* EOF */
