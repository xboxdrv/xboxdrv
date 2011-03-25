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

#include "dbus_subsystem.hpp"

#include <boost/format.hpp>
#include <dbus/dbus-glib-lowlevel.h>
//#include <dbus/dbus-glib-binding.h>
#include <dbus/dbus.h>
#include <sstream>
#include <stdexcept>

#include "raise_exception.hpp"
#include "xboxdrv_g_controller.hpp"
#include "xboxdrv_g_daemon.hpp"
#include "xboxdrv_daemon_glue.hpp"
#include "xboxdrv_controller_glue.hpp"

DBusSubsystem::DBusSubsystem(const std::string& name) :
  m_connection()
{
  GError* gerror = NULL;

  // this calls automatically sets up connection to the main loop
  m_connection = dbus_g_bus_get(DBUS_BUS_SESSION, &gerror);
  if (!m_connection)
  {
    std::ostringstream out;
    out << "failed to open connection to bus: " << gerror->message;
    g_error_free(gerror);
    throw std::runtime_error(out.str());
  }

  request_name(name);
}

DBusSubsystem::~DBusSubsystem()
{
  dbus_g_connection_unref(m_connection);
}

void
DBusSubsystem::request_name(const std::string& name)
{
  DBusError error;
  dbus_error_init(&error);

  // FIXME: replace this with org_freedesktop_DBus_request_name()
  int ret = dbus_bus_request_name(dbus_g_connection_get_connection(m_connection),
                                  name.c_str(),
                                  DBUS_NAME_FLAG_REPLACE_EXISTING,
                                  &error);
  
  if (dbus_error_is_set(&error))
  { 
    std::ostringstream out;
    out << "failed to get unique dbus name: " <<  error.message;
    dbus_error_free(&error);
    throw std::runtime_error(out.str());
  }

  if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) 
  { 
    raise_exception(std::runtime_error, "failed to become primary owner of dbus name");
  }
}

void
DBusSubsystem::register_xboxdrv_daemon(XboxdrvDaemon* c_daemon)
{
  // FIXME: should unref() these somewhere
  XboxdrvGDaemon* daemon = xboxdrv_g_daemon_new(c_daemon);
  dbus_g_object_type_install_info(XBOXDRV_TYPE_G_DAEMON, &dbus_glib_xboxdrv_daemon_object_info);
  dbus_g_connection_register_g_object(m_connection, "/org/seul/Xboxdrv/Daemon", G_OBJECT(daemon));
}

void
DBusSubsystem::register_controller_slots(const std::vector<ControllerSlotPtr>& slots)
{
  for(std::vector<ControllerSlotPtr>::const_iterator i = slots.begin(); i != slots.end(); ++i)
  {
    XboxdrvGController* controller = xboxdrv_g_controller_new(i->get());
    dbus_g_object_type_install_info(XBOXDRV_TYPE_G_CONTROLLER, &dbus_glib_xboxdrv_controller_object_info);
    dbus_g_connection_register_g_object(m_connection, 
                                        (boost::format("/org/seul/Xboxdrv/ControllerSlots/%d")
                                         % (i - slots.begin())).str().c_str(),
                                        G_OBJECT(controller));
  }
}

/* EOF */
