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

#ifndef HEADER_XBOXDRV_DBUS_SUBSYSTEM_HPP
#define HEADER_XBOXDRV_DBUS_SUBSYSTEM_HPP

#include <dbus/dbus-glib.h>
#include <string>
#include <vector>

#include "controller_slot_ptr.hpp"

namespace xboxdrv {

class XboxdrvDaemon;

class DBusSubsystem
{
private:
  DBusGConnection* m_connection;

public:
  DBusSubsystem(const std::string& name, DBusBusType bus_type);
  ~DBusSubsystem();

  void register_xboxdrv_daemon(XboxdrvDaemon* c_daemon);
  void register_controller_slots(const std::vector<ControllerSlotPtr>& slots);

private:
  void request_name(const std::string& name);

private:
  DBusSubsystem(const DBusSubsystem&);
  DBusSubsystem& operator=(const DBusSubsystem&);
};

} // namespace xboxdrv

#endif

/* EOF */
