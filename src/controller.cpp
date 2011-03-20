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

#include "controller.hpp"

#include <boost/bind.hpp>

#include "log.hpp"
#include "message_processor.hpp"

Controller::Controller() :
  m_msg_cb(),
  m_udev_device()
{
}

Controller::~Controller()
{
  udev_device_unref(m_udev_device);
}

void
Controller::submit_msg(const XboxGenericMsg& msg)
{
  if (m_msg_cb)
  {
    m_msg_cb(msg);
  }
}

void
Controller::set_udev_device(udev_device* udev_dev)
{
  m_udev_device = udev_dev;
  udev_device_ref(m_udev_device);
}

void
Controller::set_message_cb(boost::function<void(const XboxGenericMsg&)> msg_cb)
{
  m_msg_cb = msg_cb;
}

udev_device*
Controller::get_udev_device() const
{
  return m_udev_device;
}

/* EOF */
