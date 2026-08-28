/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#include <format>
#include <stdexcept>
#include <string>

#include <logmich/log.hpp>

#include "controller.hpp"
#include "controller_slot.hpp"
#include "controller_thread.hpp"
#include "message_processor.hpp"
#include "raise_exception.hpp"
#include "xboxdrv_daemon.hpp"

namespace xboxdrv {

namespace {

// Interface XML kept in sync with src/xboxdrv_daemon.xml /
// src/xboxdrv_controller.xml so introspection stays accurate.
static char const* const kDaemonXml =
  "<node>"
  "  <interface name=\"org.seul.Xboxdrv.Daemon\">"
  "    <method name=\"Status\">"
  "      <arg type=\"s\" direction=\"out\"/>"
  "    </method>"
  "    <method name=\"Shutdown\"/>"
  "    <signal name=\"ControllerConnected\">"
  "      <arg name=\"slot\" type=\"i\"/>"
  "    </signal>"
  "    <signal name=\"ControllerDisconnected\">"
  "      <arg name=\"slot\" type=\"i\"/>"
  "    </signal>"
  "  </interface>"
  "</node>";

static char const* const kControllerXml =
  "<node>"
  "  <interface name=\"org.seul.Xboxdrv.Controller\">"
  "    <method name=\"SetLed\">"
  "      <arg name=\"status\" type=\"i\" direction=\"in\"/>"
  "    </method>"
  "    <method name=\"SetRumble\">"
  "      <arg name=\"strong\" type=\"i\" direction=\"in\"/>"
  "      <arg name=\"weak\" type=\"i\" direction=\"in\"/>"
  "    </method>"
  "    <method name=\"SetConfig\">"
  "      <arg name=\"config\" type=\"i\" direction=\"in\"/>"
  "    </method>"
  "  </interface>"
  "</node>";

static void
daemon_method_call(GDBusConnection*       /*connection*/,
                   gchar const*           /*sender*/,
                   gchar const*           /*object_path*/,
                   gchar const*           /*interface_name*/,
                   gchar const*           method_name,
                   GVariant*              parameters,
                   GDBusMethodInvocation* invocation,
                   gpointer               user_data)
{
  auto* daemon = static_cast<XboxdrvDaemon*>(user_data);

  if (g_strcmp0(method_name, "Status") == 0)
  {
    log_info("D-Bus: Daemon.Status()");
    std::string const status = daemon->status();
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new("(s)", status.c_str()));
    return;
  }

  if (g_strcmp0(method_name, "Shutdown") == 0)
  {
    log_info("D-Bus: Daemon.Shutdown()");
    daemon->shutdown();
    g_dbus_method_invocation_return_value(invocation, nullptr);
    return;
  }

  g_dbus_method_invocation_return_error(invocation,
                                        G_DBUS_ERROR,
                                        G_DBUS_ERROR_UNKNOWN_METHOD,
                                        "Unknown method %s",
                                        method_name);
}

static void
controller_method_call(GDBusConnection*       /*connection*/,
                       gchar const*           /*sender*/,
                       gchar const*           /*object_path*/,
                       gchar const*           /*interface_name*/,
                       gchar const*           method_name,
                       GVariant*              parameters,
                       GDBusMethodInvocation* invocation,
                       gpointer               user_data)
{
  auto* slot = static_cast<ControllerSlot*>(user_data);

  if (g_strcmp0(method_name, "SetLed") == 0)
  {
    gint status = 0;
    g_variant_get(parameters, "(i)", &status);
    log_info("D-Bus: Controller.SetLed({})", status);

    if (slot && slot->get_controller())
    {
      slot->get_controller()->set_led(static_cast<uint8_t>(status));
      g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else
    {
      g_dbus_method_invocation_return_error(invocation,
                                            G_DBUS_ERROR,
                                            G_DBUS_ERROR_FAILED,
                                            "couldn't access controller");
    }
    return;
  }

  if (g_strcmp0(method_name, "SetRumble") == 0)
  {
    gint strong = 0;
    gint weak = 0;
    g_variant_get(parameters, "(ii)", &strong, &weak);
    log_info("D-Bus: Controller.SetRumble({}, {})", strong, weak);

    if (slot && slot->get_controller())
    {
      slot->get_controller()->set_rumble(static_cast<uint8_t>(strong),
                                         static_cast<uint8_t>(weak));
      g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else
    {
      g_dbus_method_invocation_return_error(invocation,
                                            G_DBUS_ERROR,
                                            G_DBUS_ERROR_FAILED,
                                            "couldn't access controller");
    }
    return;
  }

  if (g_strcmp0(method_name, "SetConfig") == 0)
  {
    gint config_num = 0;
    g_variant_get(parameters, "(i)", &config_num);
    log_info("D-Bus: Controller.SetConfig({})", config_num);

    if (slot &&
        slot->get_thread() &&
        slot->get_thread()->get_controller())
    {
      try
      {
        MessageProcessor* msg_proc = slot->get_thread()->get_message_proc();
        msg_proc->set_config(config_num);
        g_dbus_method_invocation_return_value(invocation, nullptr);
      }
      catch (std::exception const& err)
      {
        g_dbus_method_invocation_return_error(invocation,
                                              G_DBUS_ERROR,
                                              G_DBUS_ERROR_FAILED,
                                              "%s",
                                              err.what());
      }
    }
    else
    {
      g_dbus_method_invocation_return_error(invocation,
                                            G_DBUS_ERROR,
                                            G_DBUS_ERROR_FAILED,
                                            "couldn't access controller");
    }
    return;
  }

  g_dbus_method_invocation_return_error(invocation,
                                        G_DBUS_ERROR,
                                        G_DBUS_ERROR_UNKNOWN_METHOD,
                                        "Unknown method %s",
                                        method_name);
}

static GDBusInterfaceVTable const kDaemonVTable = {
  daemon_method_call,
  nullptr, // get_property
  nullptr, // set_property
};

static GDBusInterfaceVTable const kControllerVTable = {
  controller_method_call,
  nullptr,
  nullptr,
};

} // namespace

DBusSubsystem::DBusSubsystem(std::string const& name, GBusType bus_type) :
  m_connection(nullptr),
  m_daemon_registration_id(0),
  m_controller_registration_ids(),
  m_daemon_node_info(nullptr),
  m_controller_node_info(nullptr)
{
  GError* error = nullptr;
  m_connection = g_bus_get_sync(bus_type, nullptr, &error);
  if (!m_connection)
  {
    std::string msg = std::format("failed to open connection to bus: {}",
                                  error ? error->message : "unknown");
    g_clear_error(&error);
    throw std::runtime_error(msg);
  }

  // Attach the connection to the default GLib main context (same as
  // the old dbus-glib path).
  g_dbus_connection_set_exit_on_close(m_connection, FALSE);

  m_daemon_node_info = g_dbus_node_info_new_for_xml(kDaemonXml, &error);
  if (!m_daemon_node_info)
  {
    std::string msg = std::format("failed to parse daemon D-Bus XML: {}",
                                  error ? error->message : "unknown");
    g_clear_error(&error);
    throw std::runtime_error(msg);
  }

  m_controller_node_info = g_dbus_node_info_new_for_xml(kControllerXml, &error);
  if (!m_controller_node_info)
  {
    std::string msg = std::format("failed to parse controller D-Bus XML: {}",
                                  error ? error->message : "unknown");
    g_clear_error(&error);
    throw std::runtime_error(msg);
  }

  request_name(name);
}

DBusSubsystem::~DBusSubsystem()
{
  if (m_connection)
  {
    if (m_daemon_registration_id != 0)
    {
      g_dbus_connection_unregister_object(m_connection, m_daemon_registration_id);
    }
    for (guint id : m_controller_registration_ids)
    {
      g_dbus_connection_unregister_object(m_connection, id);
    }
    g_object_unref(m_connection);
  }

  if (m_daemon_node_info)
  {
    g_dbus_node_info_unref(m_daemon_node_info);
  }
  if (m_controller_node_info)
  {
    g_dbus_node_info_unref(m_controller_node_info);
  }
}

void
DBusSubsystem::request_name(std::string const& name)
{
  GError* error = nullptr;

  // Blocking RequestName so we fail early with the same semantics as the
  // previous dbus-glib implementation (must become primary owner).
  GVariant* reply = g_dbus_connection_call_sync(
    m_connection,
    "org.freedesktop.DBus",
    "/org/freedesktop/DBus",
    "org.freedesktop.DBus",
    "RequestName",
    g_variant_new("(su)", name.c_str(),
                  static_cast<guint32>(G_BUS_NAME_OWNER_FLAGS_REPLACE)),
    G_VARIANT_TYPE("(u)"),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    nullptr,
    &error);

  if (!reply)
  {
    std::string msg = std::format("failed to get unique dbus name: {}",
                                  error ? error->message : "unknown");
    g_clear_error(&error);
    throw std::runtime_error(msg);
  }

  guint32 request_result = 0;
  g_variant_get(reply, "(u)", &request_result);
  g_variant_unref(reply);

  // 1 == primary owner (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
  if (request_result != 1)
  {
    raise_exception(std::runtime_error,
                    "failed to become primary owner of dbus name");
  }
}

void
DBusSubsystem::register_xboxdrv_daemon(XboxdrvDaemon* daemon)
{
  GError* error = nullptr;
  m_daemon_registration_id = g_dbus_connection_register_object(
    m_connection,
    "/org/seul/Xboxdrv/Daemon",
    m_daemon_node_info->interfaces[0],
    &kDaemonVTable,
    daemon,
    nullptr, // user_data_free
    &error);

  if (m_daemon_registration_id == 0)
  {
    std::string msg = std::format("failed to register Daemon object: {}",
                                  error ? error->message : "unknown");
    g_clear_error(&error);
    throw std::runtime_error(msg);
  }
}

void
DBusSubsystem::register_controller_slots(std::vector<ControllerSlotPtr> const& slots)
{
  for (size_t i = 0; i < slots.size(); ++i)
  {
    std::string path = std::format("/org/seul/Xboxdrv/ControllerSlots/{}", i);
    GError* error = nullptr;
    guint id = g_dbus_connection_register_object(
      m_connection,
      path.c_str(),
      m_controller_node_info->interfaces[0],
      &kControllerVTable,
      slots[i].get(),
      nullptr,
      &error);

    if (id == 0)
    {
      std::string msg = std::format("failed to register Controller object {}: {}",
                                    path, error ? error->message : "unknown");
      g_clear_error(&error);
      throw std::runtime_error(msg);
    }
    m_controller_registration_ids.push_back(id);
  }
}


void
DBusSubsystem::emit_controller_connected(int slot_id)
{
  emit_slot_signal("ControllerConnected", slot_id);
}

void
DBusSubsystem::emit_controller_disconnected(int slot_id)
{
  emit_slot_signal("ControllerDisconnected", slot_id);
}

void
DBusSubsystem::emit_slot_signal(char const* signal_name, int slot_id)
{
  if (!m_connection || m_daemon_registration_id == 0)
  {
    return;
  }

  GError* error = nullptr;
  gboolean ok = g_dbus_connection_emit_signal(
    m_connection,
    nullptr, // destination (broadcast)
    "/org/seul/Xboxdrv/Daemon",
    "org.seul.Xboxdrv.Daemon",
    signal_name,
    g_variant_new("(i)", slot_id),
    &error);

  if (!ok)
  {
    log_warn("D-Bus: failed to emit {}: {}",
             signal_name,
             error ? error->message : "unknown");
    g_clear_error(&error);
  }
  else
  {
    log_info("D-Bus: emitted {}({})", signal_name, slot_id);
  }
}

} // namespace xboxdrv

/* EOF */
