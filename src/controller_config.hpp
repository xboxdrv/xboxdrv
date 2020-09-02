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

#ifndef HEADER_XBOXDRV_CONTROLLER_CONFIG_HPP
#define HEADER_XBOXDRV_CONTROLLER_CONFIG_HPP

#include "event_emitter.hpp"
#include "modifier.hpp"

class ControllerOptions;
class ControllerConfig;
typedef std::shared_ptr<ControllerConfig> ControllerConfigPtr;

class ControllerConfig
{
private:
  std::vector<ModifierPtr> m_modifier;
  EventEmitter m_emitter;

public:
  ControllerConfig(UInput& uinput, int slot, bool extra_devices,
                   const ControllerOptions& opts);

  std::vector<ModifierPtr>& get_modifier();
  EventEmitter& get_emitter();

private:
  ControllerConfig(const ControllerConfig&);
  ControllerConfig& operator=(const ControllerConfig&);
};

#endif

/* EOF */
