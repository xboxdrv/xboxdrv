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

#include "xboxdrv_g_controller.hpp"

#include "controller.hpp"
#include "controller_slot.hpp"
#include "controller_thread.hpp"
#include "uinput_message_processor.hpp"
#include "log.hpp"

#define XBOXDRV_CONTROLLER_ERROR xboxdrv_controller_error_quark()
#define XBOXDRV_CONTROLLER_ERROR_FAILED 0
GQuark
xboxdrv_controller_error_quark()
{
  return g_quark_from_static_string("xboxdrv-controller-error-quark");
}

/* will create xboxdrv_g_controller_get_type and set xboxdrv_g_controller_parent_class */
G_DEFINE_TYPE(XboxdrvGController, xboxdrv_g_controller, G_TYPE_OBJECT)

static GObject*
xboxdrv_g_controller_constructor(GType                  gtype,
                                 guint                  n_properties,
                                 GObjectConstructParam* properties)
{
  // Always chain up to the parent constructor
  GObjectClass* parent_class = G_OBJECT_CLASS(xboxdrv_g_controller_parent_class);
  return parent_class->constructor(gtype, n_properties, properties);
}

static void
xboxdrv_g_controller_class_init(XboxdrvGControllerClass* klass)
{
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->constructor = xboxdrv_g_controller_constructor;
}

static void
xboxdrv_g_controller_init(XboxdrvGController* self)
{
  self->controller = NULL;
}

XboxdrvGController*
xboxdrv_g_controller_new(ControllerSlot* controller)
{
  XboxdrvGController* self = static_cast<XboxdrvGController*>(g_object_new(XBOXDRV_TYPE_G_CONTROLLER, NULL));
  self->controller = controller;
  return self;
}

gboolean
xboxdrv_g_controller_set_led(XboxdrvGController* self, int status, GError** error)
{
  log_info("D-Bus: xboxdrv_g_controller_set_led(" << self << ", " << status << ")");

  if (self->controller &&
      self->controller->get_thread() &&
      self->controller->get_thread()->get_controller())
  {
    self->controller->get_thread()->get_controller()->set_led(status);
    return TRUE;
  }
  else
  {
    g_set_error(error, XBOXDRV_CONTROLLER_ERROR, XBOXDRV_CONTROLLER_ERROR_FAILED,
                "could't access controller");
    return FALSE;
  }
}

gboolean
xboxdrv_g_controller_set_rumble(XboxdrvGController* self, int strong, int weak, GError** error)
{
  log_info("D-Bus: xboxdrv_g_controller_set_rumble(" << self << ", " << strong << ", " << weak << ")");

  if (self->controller &&
      self->controller->get_thread() &&
      self->controller->get_thread()->get_controller())
  {
    self->controller->get_thread()->get_controller()->set_rumble(strong, weak);
    return TRUE;
  }
  else
  {
    g_set_error(error, XBOXDRV_CONTROLLER_ERROR, XBOXDRV_CONTROLLER_ERROR_FAILED,
                "could't access controller");
    return FALSE;
  }
}

gboolean
xboxdrv_g_controller_set_config(XboxdrvGController* self, int config_num, GError** error)
{
  log_info("D-Bus: xboxdrv_g_controller_set_config(" << self << ", " << config_num << ")");

  if (self->controller &&
      self->controller->get_thread() &&
      self->controller->get_thread()->get_controller())
  {
    MessageProcessor* gen_msg_proc = self->controller->get_thread()->get_message_proc();
    UInputMessageProcessor* msg_proc = dynamic_cast<UInputMessageProcessor*>(gen_msg_proc);

    try 
    {
      msg_proc->set_config(config_num);
      return TRUE;
    }
    catch(const std::exception& err)
    {
      g_set_error(error, XBOXDRV_CONTROLLER_ERROR, XBOXDRV_CONTROLLER_ERROR_FAILED,
                  "%s", err.what());
      return FALSE;
    }
  }
  else
  {
    g_set_error(error, XBOXDRV_CONTROLLER_ERROR, XBOXDRV_CONTROLLER_ERROR_FAILED,
                "could't access controller");
    return FALSE;
  }
}

/* EOF */
