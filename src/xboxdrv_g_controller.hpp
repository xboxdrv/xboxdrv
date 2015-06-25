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

#ifndef HEADER_XBOXDRV_XBOXDRV_G_CONTROLLER_HPP
#define HEADER_XBOXDRV_XBOXDRV_G_CONTROLLER_HPP

#include <glib-object.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"

class ControllerSlot;

#define XBOXDRV_TYPE_G_CONTROLLER                  (xboxdrv_g_controller_get_type ())
#define XBOXDRV_G_CONTROLLER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), XBOXDRV_TYPE_G_CONTROLLER, XboxdrvGController))
#define XBOXDRV_IS_G_CONTROLLER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBOXDRV_TYPE_G_CONTROLLER))
#define XBOXDRV_G_CONTROLLER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), XBOXDRV_TYPE_G_CONTROLLER, XboxdrvGControllerClass))
#define XBOXDRV_IS_G_CONTROLLER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), XBOXDRV_TYPE_G_CONTROLLER))
#define XBOXDRV_G_CONTROLLER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), XBOXDRV_TYPE_G_CONTROLLER, XboxdrvGControllerClass))

typedef struct _XboxdrvGController        XboxdrvGController;
typedef struct _XboxdrvGControllerClass   XboxdrvGControllerClass;

struct _XboxdrvGController
{
  GObject parent_instance;

  ControllerSlot* controller;
};

struct _XboxdrvGControllerClass
{
  GObjectClass parent_class;
};

GType xboxdrv_g_controller_get_type();
XboxdrvGController* xboxdrv_g_controller_new(ControllerSlot* controller);

gboolean xboxdrv_g_controller_set_config(XboxdrvGController* self, int config_num, GError** error);
gboolean xboxdrv_g_controller_set_led(XboxdrvGController* self, int status, GError** error);
gboolean xboxdrv_g_controller_set_rumble(XboxdrvGController* self, int strong, int weak, GError** error);

#endif

/* EOF */
