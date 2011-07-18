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
#include <linux/input.h>
#include <errno.h>
#include <stdexcept>

#include "log.hpp"
#include "virtualkeyboard/virtual_keyboard.hpp"

KeyboardController::KeyboardController(VirtualKeyboard& keyboard, const std::string& device) :
  m_keyboard(keyboard),
  m_device(device),
  m_fd(-1),
  m_io_channel(0)
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
    
  guint source_id;
  source_id = g_io_add_watch(m_io_channel, 
                             static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                             &KeyboardController::on_read_data_wrap, this);
}

KeyboardController::~KeyboardController()
{
  g_io_channel_unref(m_io_channel);
  close(m_fd);
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
  }
  else if (ev.type == EV_KEY)
  {
    if (ev.code == BTN_A)
    {
      m_keyboard.send_key(ev.value);
    }
  }
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
  
  return TRUE;
}

/* EOF */
