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

#include "controller_slot.hpp"

#include <boost/format.hpp>

#include "xboxdrv_thread.hpp"

void
ControllerSlot::connect(XboxdrvThread* thread,
                        uint8_t busnum, uint8_t devnum,
                        const XPadDevice& dev_type)
{
  assert(m_thread == 0);
  m_thread = thread;

  m_busnum = busnum;
  m_devnum = devnum;
  m_dev_type = dev_type;
}

void
ControllerSlot::disconnect()
{
  delete m_thread;
  m_thread = 0;
}

bool
ControllerSlot::try_disconnect()
{
  assert(m_thread);

  if (m_thread->try_join_thread())
  {
    disconnect();
    return true;
  }
  else
  {
    return false;
  }
}

bool
ControllerSlot::is_connected() const
{
  return m_thread;
}

std::string
ControllerSlot::get_usbpath() const
{
  return (boost::format("%03d:%03d") % static_cast<int>(m_busnum) % static_cast<int>(m_devnum)).str();
}

std::string
ControllerSlot::get_usbid() const
{
  return (boost::format("%04x:%04x") % m_dev_type.idVendor % m_dev_type.idProduct).str();
}

std::string
ControllerSlot::get_name() const
{
  return m_dev_type.name;
}

/* EOF */
