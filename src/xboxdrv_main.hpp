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

#ifndef HEADER_XBOXDRV_XBOXDRV_MAIN_HPP
#define HEADER_XBOXDRV_XBOXDRV_MAIN_HPP

#include <memory>
#include <libusb.h>
#include <glib.h>
#include <boost/scoped_ptr.hpp>

#include "xpad_device.hpp"
#include "controller_ptr.hpp"

class MessageProcessor;
class Options;
class UInput;
class USBGSource;

class XboxdrvMain
{
private:
  static XboxdrvMain* s_current;

public:
  static XboxdrvMain* current() { return s_current; }

private:
  const Options& m_opts;
  GMainLoop* m_gmain;
  boost::scoped_ptr<USBGSource> m_usb_gsource;

  std::auto_ptr<UInput> m_uinput;

  int m_jsdev_number;
  int m_evdev_number;
  bool m_use_libusb;

  XPadDevice m_dev_type;

  ControllerPtr m_controller;

public:
  XboxdrvMain(const Options& opts);
  ~XboxdrvMain();

  void run();
  void shutdown();

private:
  ControllerPtr create_controller();

  void init_controller(const ControllerPtr& controller);

  void print_info(libusb_device* dev,
                  const XPadDevice& dev_type,
                  const Options& opts) const;

  static void on_sigint(int);

  void on_controller_disconnect();

  void on_child_watch(GPid pid, gint status);
  static void on_child_watch_wrap(GPid pid, gint status, gpointer data) {
    static_cast<XboxdrvMain*>(data)->on_child_watch(pid, status);
  }

private:
  XboxdrvMain(const XboxdrvMain&);
  XboxdrvMain& operator=(const XboxdrvMain&);
};

#endif

/* EOF */
