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

#include "controller_config.hpp"

#include "controller_options.hpp"

ControllerConfig::ControllerConfig(UInput& uinput, int slot, bool extra_devices, const ControllerOptions& opts) :
  m_modifier(),
  m_uinput(uinput, slot, extra_devices, opts.uinput)
{
}

std::vector<ModifierPtr>&
ControllerConfig::get_modifier()
{
  return m_modifier;
}

UInputConfig&
ControllerConfig::get_uinput()
{
  return m_uinput;
}

/* EOF */
