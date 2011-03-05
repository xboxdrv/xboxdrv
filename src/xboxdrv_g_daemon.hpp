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

#ifndef HEADER_XBOXDRV_XBOXDRV_G_DAEMON_HPP
#define HEADER_XBOXDRV_XBOXDRV_G_DAEMON_HPP

#include <glib-object.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"

class XboxdrvDaemon;

#define XBOXDRV_TYPE_G_DAEMON                  (xboxdrv_g_daemon_get_type ())
#define XBOXDRV_G_DAEMON(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), XBOXDRV_TYPE_G_DAEMON, XboxdrvGDaemon))
#define XBOXDRV_IS_G_DAEMON(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBOXDRV_TYPE_G_DAEMON))
#define XBOXDRV_G_DAEMON_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), XBOXDRV_TYPE_G_DAEMON, XboxdrvGDaemonClass))
#define XBOXDRV_IS_G_DAEMON_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), XBOXDRV_TYPE_G_DAEMON))
#define XBOXDRV_G_DAEMON_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), XBOXDRV_TYPE_G_DAEMON, XboxdrvGDaemonClass))

typedef struct _XboxdrvGDaemon        XboxdrvGDaemon;
typedef struct _XboxdrvGDaemonClass   XboxdrvGDaemonClass;

struct _XboxdrvGDaemon
{
  GObject parent_instance;

  XboxdrvDaemon* daemon;
};

struct _XboxdrvGDaemonClass
{
  GObjectClass parent_class;
};

GType xboxdrv_g_daemon_get_type();
XboxdrvGDaemon* xboxdrv_g_daemon_new(XboxdrvDaemon* daemon);

gboolean xboxdrv_g_daemon_status(XboxdrvGDaemon* self, gchar** ret, GError** error);
gboolean xboxdrv_g_daemon_shutdown(XboxdrvGDaemon* self, GError** error);

#endif

/* EOF */
