/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "uinput_options.hpp"

#include <linux/input.h>

#include "button_event_factory.hpp"

namespace xboxdrv {

UInputOptions::UInputOptions() :
  m_btn_map(),
  m_axis_map()
{
  set_defaults();
}

ButtonMapOptions&
UInputOptions::get_btn_map()
{
  return m_btn_map;
}

AxisMapOptions&
UInputOptions::get_axis_map()
{
  return m_axis_map;
}

const ButtonMapOptions&
UInputOptions::get_btn_map() const
{
  return m_btn_map;
}

const AxisMapOptions&
UInputOptions::get_axis_map() const
{
  return m_axis_map;
}

void
UInputOptions::mimic_xpad()
{
  // device_name is set in Options::set_mimic_xpad()
  get_axis_map().clear();
  get_btn_map().clear();

  m_btn_map.push_back(ButtonMapOption("gamepad.start", "BTN_START"));
  m_btn_map.push_back(ButtonMapOption("gamepad.guide", "BTN_MODE"));
  m_btn_map.push_back(ButtonMapOption("gamepad.back",  "BTN_SELECT"));

  m_btn_map.push_back(ButtonMapOption("gamepad.a",    "BTN_A"));
  m_btn_map.push_back(ButtonMapOption("gamepad.b",    "BTN_B"));
  m_btn_map.push_back(ButtonMapOption("gamepad.x",    "BTN_X"));
  m_btn_map.push_back(ButtonMapOption("gamepad.y",    "BTN_Y"));

  m_btn_map.push_back(ButtonMapOption("gamepad.lb",    "BTN_TL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.rb",    "BTN_TR"));

  m_btn_map.push_back(ButtonMapOption("gamepad.tl",    "BTN_THUMBL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.tr",    "BTN_THUMBR"));


  m_axis_map.push_back(AxisMapOption("gamepad.x1", "ABS_X:-32768:32767:16:128"));
  m_axis_map.push_back(AxisMapOption("gamepad.y1", "ABS_Y:-32768:32767:16:128"));

  m_axis_map.push_back(AxisMapOption("gamepad.x2", "ABS_RX:-32768:32767:16:128"));
  m_axis_map.push_back(AxisMapOption("gamepad.y2", "ABS_RY:-32768:32767:16:128"));

  m_axis_map.push_back(AxisMapOption("gamepad.lt", "ABS_Z:0:255:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.rt", "ABS_RZ:0:255:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.dpad_x", "ABS_HAT0X:-1:1:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.dpad_y", "ABS_HAT0Y:-1:1:0:0"));
}

void
UInputOptions::mimic_xpad_wireless()
{
  // device_name is set in Options::set_mimic_xpad_wireless()
  get_axis_map().clear();
  get_btn_map().clear();

  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_up",    "BTN_TRIGGER_HAPPY3"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_down",  "BTN_TRIGGER_HAPPY4"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_left",  "BTN_TRIGGER_HAPPY1"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_right", "BTN_TRIGGER_HAPPY2"));

  m_btn_map.push_back(ButtonMapOption("gamepad.start", "BTN_START"));
  m_btn_map.push_back(ButtonMapOption("gamepad.guide", "BTN_MODE"));
  m_btn_map.push_back(ButtonMapOption("gamepad.back",  "BTN_SELECT"));

  m_btn_map.push_back(ButtonMapOption("gamepad.a",    "BTN_A"));
  m_btn_map.push_back(ButtonMapOption("gamepad.b",    "BTN_B"));
  m_btn_map.push_back(ButtonMapOption("gamepad.x",    "BTN_X"));
  m_btn_map.push_back(ButtonMapOption("gamepad.y",    "BTN_Y"));

  m_btn_map.push_back(ButtonMapOption("gamepad.lb",    "BTN_TL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.rb",    "BTN_TR"));

  m_btn_map.push_back(ButtonMapOption("gamepad.tl",    "BTN_THUMBL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.tr",    "BTN_THUMBR"));

  m_axis_map.push_back(AxisMapOption("gamepad.x1", "ABS_X:-32768:32767:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.y1", "ABS_Y:-32768:32767:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.x2", "ABS_RX:-32768:32767:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.y2", "ABS_RY:-32768:32767:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.lt", "ABS_Z:0:255:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.rt", "ABS_RZ:0:255:0:0"));
}

void
UInputOptions::set_defaults()
{
  get_btn_map().clear();
  get_axis_map().clear();

  m_btn_map.push_back(ButtonMapOption("gamepad.start", "BTN_START"));
  m_btn_map.push_back(ButtonMapOption("gamepad.guide", "BTN_MODE"));
  m_btn_map.push_back(ButtonMapOption("gamepad.back", "BTN_SELECT"));

  m_btn_map.push_back(ButtonMapOption("gamepad.a",    "BTN_A"));
  m_btn_map.push_back(ButtonMapOption("gamepad.b",    "BTN_B"));
  m_btn_map.push_back(ButtonMapOption("gamepad.x",    "BTN_X"));
  m_btn_map.push_back(ButtonMapOption("gamepad.y",    "BTN_Y"));

  m_btn_map.push_back(ButtonMapOption("gamepad.lb",    "BTN_TL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.rb",    "BTN_TR"));

  m_btn_map.push_back(ButtonMapOption("gamepad.tl",    "BTN_THUMBL"));
  m_btn_map.push_back(ButtonMapOption("gamepad.tr",    "BTN_THUMBR"));

  m_axis_map.push_back(AxisMapOption("gamepad.x1", "ABS_X:-32768:32767:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.y1", "ABS_Y:-32768:32767:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.x2", "ABS_RX:-32768:32767:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.y2", "ABS_RY:-32768:32767:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.lt", "ABS_BRAKE:0:255:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.rt", "ABS_GAS:0:255:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.dpad_x", "ABS_HAT0X:-1:1:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.dpad_y", "ABS_HAT0Y:-1:1:0:0"));
}

void
UInputOptions::trigger_as_button()
{
  m_axis_map.push_back(AxisMapOption("gamepad.lt", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.rt", "void"));

  m_btn_map.push_back(ButtonMapOption("gamepad.lt",    "BTN_TL2"));
  m_btn_map.push_back(ButtonMapOption("gamepad.rt",    "BTN_TR2"));
}

void
UInputOptions::trigger_as_zaxis()
{
  m_axis_map.push_back(AxisMapOption("gamepad.trigger", "ABS_Z:-255:255:0:0"));

  m_axis_map.push_back(AxisMapOption("gamepad.lt", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.rt", "void"));
}

void
UInputOptions::dpad_as_button()
{
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_up",    "BTN_BASE"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_down",  "BTN_BASE2"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_left",  "BTN_BASE3"));
  m_btn_map.push_back(ButtonMapOption("gamepad.dpad_right", "BTN_BASE4"));

  m_axis_map.push_back(AxisMapOption("gamepad.dpad_x", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.dpad_y", "void"));
}

void
UInputOptions::dpad_only()
{
  m_axis_map.push_back(AxisMapOption("gamepad.x1", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.y1", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.x2", "void"));
  m_axis_map.push_back(AxisMapOption("gamepad.y2", "void"));

  m_axis_map.push_back(AxisMapOption("gamepad.dpad_x", "ABS_X:-1:1:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.dpad_y", "ABS_Y:-1:1:0:0"));
}

void
UInputOptions::guitar()
{
  get_btn_map().clear();
  get_axis_map().clear();

  m_btn_map.push_back(ButtonMapOption("gamepad.start", "BTN_START"));
  m_btn_map.push_back(ButtonMapOption("gamepad.guide", "BTN_MODE"));
  m_btn_map.push_back(ButtonMapOption("gamepad.back", "BTN_SELECT"));

  m_btn_map.push_back(ButtonMapOption("gamepad.a", "BTN_0"));  // green
  m_btn_map.push_back(ButtonMapOption("gamepad.b", "BTN_1"));  // red
  m_btn_map.push_back(ButtonMapOption("gamepad.y", "BTN_2"));  // blue
  m_btn_map.push_back(ButtonMapOption("gamepad.x", "BTN_3"));  // yellow
  m_btn_map.push_back(ButtonMapOption("gamepad.lb", "BTN_4")); // orange

  m_axis_map.push_back(AxisMapOption("gamepad.x2", "ABS_X:-32768:32767:0:0")); // whammy
  m_axis_map.push_back(AxisMapOption("gamepad.y2", "ABS_Y:-32768:32767:0:0")); // tilt

  m_axis_map.push_back(AxisMapOption("gamepad.dpad_x", "ABS_HAT0X:-1:1:0:0"));
  m_axis_map.push_back(AxisMapOption("gamepad.dpad_y", "ABS_HAT0Y:-1:1:0:0"));
}

} // namespace xboxdrv

/* EOF */
