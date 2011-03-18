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

#include "usb_system.hpp"

#include <assert.h>
#include <poll.h>
#include <libusb.h>
#include <stdlib.h>

#include "log.hpp"

// GUSBSource

// documentation at: http://library.gnome.org/devel/glib/2.28/
USBSystem::USBSystem() :
  m_source_funcs(),
  m_source(),
  m_source_id()
{
  // create the source functions
  m_source_funcs.prepare  = &USBSystem::on_source_prepare;
  m_source_funcs.check    = &USBSystem::on_source_check;
  m_source_funcs.dispatch = &USBSystem::on_source_dispatch;
  m_source_funcs.finalize = NULL;

  m_source_funcs.closure_callback = NULL;
  m_source_funcs.closure_marshal  = NULL;

  // create the source itself
  m_source = g_source_new(&m_source_funcs, sizeof(GSource));
  g_source_set_callback(m_source,
                        &USBSystem::on_source_wrap, this,
                        NULL);

  // add pollfds to source
  const libusb_pollfd** fds = libusb_get_pollfds(NULL);
  for(const libusb_pollfd** i = fds; *i != NULL; ++i)
  {
    log_debug("adding pollfd: " << (*i)->fd);
    GPollFD* gfd = new GPollFD;
    
    gfd->fd = (*i)->fd;
    gfd->events  = (*i)->events;
    gfd->revents = 0;
    
    g_source_add_poll(m_source, gfd);
  }
  free(fds);

  // register pollfd callbacks
  libusb_set_pollfd_notifiers(NULL, 
                              &USBSystem::on_usb_pollfd_added_wrap,
                              &USBSystem::on_usb_pollfd_removed_wrap,
                              this);
}

USBSystem::~USBSystem()
{
}

void
USBSystem::attach(GMainContext* context)
{
  // attach source
  m_source_id = g_source_attach(m_source, context);
}

void
USBSystem::on_usb_pollfd_added(int fd, short events)
{
  assert(POLLIN  == G_IO_IN);
  assert(POLLOUT == G_IO_OUT);

  log_trace();
  GPollFD* gfd = new GPollFD;

  gfd->fd = fd;
  gfd->events  = events;
  gfd->revents = 0;

  g_source_add_poll(m_source, gfd);

  // FIXME: put gfd somewhere, like a map or list
}

void
USBSystem::on_usb_pollfd_removed(int fd)
{
  log_trace();
  //  FIXME: how to get the GPollFD
  //g_source_remove_poll(m_source, &gfd);
}

gboolean
USBSystem::on_source_prepare(GSource* source, gint* timeout)
{
  log_trace();

  struct timeval tv;
  int ret = libusb_get_next_timeout(NULL, &tv);

  log_debug("libusb_get_next_timeout(): " << ret);

  if (ret == LIBUSB_SUCCESS)
  {
    // no timeout
  }
  else
  {
    // convert tv
    *timeout = -1;
  }

  return TRUE;
}

gboolean
USBSystem::on_source_check(GSource* source)
{
  log_trace();
  return TRUE;
}

gboolean
USBSystem::on_source_dispatch(GSource* source, GSourceFunc callback, gpointer userdata)
{
  log_trace();
  return callback(userdata);
}

gboolean
USBSystem::on_source()
{
  log_trace();
  libusb_handle_events(NULL);
  return TRUE;
}

/* EOF */
