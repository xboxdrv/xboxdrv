/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_FWD_HPP
#define HEADER_XBOXDRV_FWD_HPP

namespace xboxdrv {

class AxisEvent;
class AxisEventHandler;
class AxisFilter;
class AxisMap;
class AxismapModifier;
class ButtonEvent;
class ButtonEventHandler;
class ButtonFilter;
class ButtonmapModifier;
class ButtonMapping;
class Chatpad;
class Controller;
class ControllerConfig;
class ControllerMatchRule;
class ControllerMessage;
class ControllerMessageDescriptor;
class ControllerOptions;
class ControllerSlot;
class ControllerSlotConfig;
class ControllerSlotOptions;
class ControllerThread;
class CycleKeySequence;
class Headset;
class MessageProcessor;
class Modifier;
class Namespace;
class Options;
class Symbol;
class UInputOptions;
class USBGSource;
class USBSubsystem;
class Xboxdrv;
class XboxdrvDaemon;

} // namespace xboxdrv

#endif

/* EOF */
