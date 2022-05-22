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

#include "xboxdrv_g_daemon.hpp"

#include <logmich/log.hpp>

#include "xboxdrv_daemon.hpp"

#pragma GCC diagnostic ignored "-Wcast-qual"

/* will create xboxdrv_g_daemon_get_type and set xboxdrv_g_daemon_parent_class */
G_DEFINE_TYPE(XboxdrvGDaemon, xboxdrv_g_daemon, G_TYPE_OBJECT)

static GObject*
xboxdrv_g_daemon_constructor(GType                  gtype,
                             guint                  n_properties,
                             GObjectConstructParam* properties)
{
  // Always chain up to the parent constructor
  GObjectClass* parent_class = G_OBJECT_CLASS(xboxdrv_g_daemon_parent_class);
  return parent_class->constructor(gtype, n_properties, properties);
}

static void
xboxdrv_g_daemon_class_init(XboxdrvGDaemonClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->constructor = xboxdrv_g_daemon_constructor;
}

static void
xboxdrv_g_daemon_init(XboxdrvGDaemon* self)
{
  self->daemon = NULL;
}

XboxdrvGDaemon*
xboxdrv_g_daemon_new(XboxdrvDaemon* daemon)
{
  XboxdrvGDaemon* self = static_cast<XboxdrvGDaemon*>(g_object_new(XBOXDRV_TYPE_G_DAEMON, NULL));
  self->daemon = daemon;
  return self;
}

gboolean
xboxdrv_g_daemon_status(XboxdrvGDaemon* self, gchar** ret, GError** error)
{
  log_info("D-Bus: xboxdrv_g_daemon_status({})", static_cast<void*>(self));

  *ret = g_strdup(self->daemon->status().c_str());
  return TRUE;
}

gboolean xboxdrv_g_daemon_shutdown(XboxdrvGDaemon* self, GError** error)
{
  log_info("D-Bus: xboxdrv_g_daemon_shutdown({})", static_cast<void*>(self));

  self->daemon->shutdown();
  return TRUE;
}

/* EOF */
