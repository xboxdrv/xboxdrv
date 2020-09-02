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

#include <gtk/gtk.h>
#include <iostream>
#include <boost/bind.hpp>
#include <errno.h>

#include "arg_parser.hpp"
#include "log.hpp"
#include "status_icon.hpp"
#include "uinput/ui_key_event_emitter.hpp"
#include "uinput/uinput.hpp"
#include "virtualkeyboard/virtual_keyboard.hpp"
#include "virtualkeyboard/keyboard_dispatcher.hpp"
#include "virtualkeyboard/keyboard_controller.hpp"

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

  StatusIcon status_icon;
  KeyboardDescriptionPtr keyboard_desc = KeyboardDescription::create_us_layout();
  VirtualKeyboard virtual_keyboard(keyboard_desc);

  if (!device.empty())
  {
    UInput uinput(false);
    KeyboardDispatcher dispatcher(virtual_keyboard, uinput);
    KeyboardController controller(virtual_keyboard, uinput, device);
    uinput.finish();

    virtual_keyboard.show();
    gtk_main();
  }
  else
  {
    {
      std::cout << "--device DEVICE option not given, starting in test mode" << std::endl;

      // non-interactive test mode
      virtual_keyboard.show();
      gtk_main();
    }

    return EXIT_SUCCESS;
  }
}

/* EOF */
