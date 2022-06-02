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

#ifndef HEADER_UNSEBU_USB_GSOURCE_HPP
#define HEADER_UNSEBU_USB_GSOURCE_HPP

#include <glib.h>
#include <list>

namespace unsebu {

class USBGSource;

struct GUSBSource
{
  GSource source;
  USBGSource* usb_source;
};

class USBGSource
{
private:
  GSourceFuncs m_source_funcs;
  GUSBSource* m_source;
  gint m_source_id;
  std::list<GPollFD*> m_pollfds;

public:
  USBGSource();
  ~USBGSource();

  void attach(GMainContext* context);

private:
  gboolean on_source();

  // libusb callbacks
  void on_usb_pollfd_added(int fd, short events);
  void on_usb_pollfd_removed(int fd);

  static gboolean on_source_wrap(void* userdata) {
    return static_cast<USBGSource*>(userdata)->on_source();
  }

  static void on_usb_pollfd_added_wrap(int fd, short events, void* userdata) {
    static_cast<USBGSource*>(userdata)->on_usb_pollfd_added(fd, events);
  }

  static void on_usb_pollfd_removed_wrap(int fd,  void* userdata) {
    static_cast<USBGSource*>(userdata)->on_usb_pollfd_removed(fd);
  }

  // glib callbacks
  static gboolean on_source_prepare(GSource* source, gint* timeout_);
  static gboolean on_source_check(GSource* source);
  static gboolean on_source_dispatch(GSource* source, GSourceFunc callback, gpointer userdata);

private:
  USBGSource(const USBGSource&);
  USBGSource& operator=(const USBGSource&);
};

} // namespace unsebu

#endif

/* EOF */
