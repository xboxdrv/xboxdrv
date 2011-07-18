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

#include <gtk/gtk.h>
#include <iostream>
#include <boost/bind.hpp>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "arg_parser.hpp"
#include "log.hpp"
#include "ui_key_event_emitter.hpp"
#include "uinput.hpp"
#include "virtual_keyboard.hpp"

class KeyboardDispatcher
{
private:
  UInput& m_uinput;
  std::vector<UIEventEmitterPtr> m_emitter;

public:
  KeyboardDispatcher(VirtualKeyboard& gui_keyboard,
                     UInput& uinput) :
    m_uinput(uinput),
    m_emitter(KEY_CNT)
  {
    const KeyboardDescription& desc = gui_keyboard.get_description();

    for(int y = 0; y < desc.get_height(); ++y)
    {
      for(int x = 0; x < desc.get_width(); ++x)
      {
        const Key& key = desc.get_key(x, y);
        if (key.m_code != -1)
        {
          m_emitter[key.m_code] = uinput.add_key(0, key.m_code);
        }
      }
    }

    gui_keyboard.set_key_callback(boost::bind(&KeyboardDispatcher::on_key, this, _1, _2));
  }

  void on_key(const Key& key, bool pressed)
  {
    if (key.m_code != -1)
    {
      log_tmp("emitting: " << key.m_code << " " << pressed);
      m_emitter[key.m_code]->send(pressed);
      m_uinput.sync();
    }
  }
};

class KeyboardController
{
private:
  VirtualKeyboard& m_keyboard;
  std::string m_device;
  int m_fd;
  GIOChannel* m_io_channel;

public:
  KeyboardController(VirtualKeyboard& keyboard, const std::string& device) :
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

  ~KeyboardController()
  {
    g_io_channel_unref(m_io_channel);
    close(m_fd);
  }

  void parse(const struct input_event& ev)
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
  on_read_data(GIOChannel* source, GIOCondition condition)
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

  static gboolean on_read_data_wrap(GIOChannel* source,
                                    GIOCondition condition,
                                    gpointer userdata) 
  {
    return static_cast<KeyboardController*>(userdata)->on_read_data(source, condition);
  }

private:
  KeyboardController(const KeyboardController&);
  KeyboardController& operator=(const KeyboardController&);
};

int main(int argc, char** argv)
{
  enum {
    OPTION_HELP,
    OPTION_DEVICE
  };

  gtk_init(&argc, &argv);

  ArgParser argp;
  argp
    .add_usage("[OPTION]...")
    .add_text("Virtual Keyboard")
    .add_newline()
 
    .add_option(OPTION_HELP,   'h', "help", "", "display this help and exit")
    .add_option(OPTION_DEVICE, 'd', "device", "DEVICE", "read events from device");

  ArgParser::ParsedOptions parsed = argp.parse_args(argc, argv);
  std::string device;
  for(ArgParser::ParsedOptions::const_iterator i = parsed.begin(); i != parsed.end(); ++i)
  {
    const ArgParser::ParsedOption& opt = *i;

    switch (i->key)
    {
      case OPTION_HELP:
        argp.print_help(std::cout);
        exit(EXIT_SUCCESS);
        break;

      case OPTION_DEVICE:
        device = opt.argument;
        break;
    }
  }

  if (device.empty())
  {
    std::cerr << "Error: --device DEVICE option must be given" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    UInput uinput(false);

    KeyboardDescription keyboard_desc(KeyboardDescription::create_us_layout()); 
    VirtualKeyboard virtual_keyboard(keyboard_desc);
    KeyboardDispatcher dispatcher(virtual_keyboard, uinput);

    KeyboardController controller(virtual_keyboard, device);

    uinput.finish();

    virtual_keyboard.show();

    gtk_main();

    return EXIT_SUCCESS;
  }
}

/* EOF */
