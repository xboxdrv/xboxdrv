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

#include "controller_config.hpp"

#include "axisfilter/deadzone_axis_filter.hpp"
#include "controller_options.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/compat_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

namespace xboxdrv {

namespace {

void
add_axis_filters(std::vector<ModifierPtr>& modifiers,
                 std::map<std::string, AxisFilterPtr> const& filters)
{
  if (filters.empty())
  {
    return;
  }

  std::shared_ptr<AxismapModifier> axismap(new AxismapModifier);
  for(std::map<std::string, AxisFilterPtr>::const_iterator i = filters.begin();
      i != filters.end(); ++i)
  {
    axismap->add_filter(i->first, i->second);
  }
  modifiers.push_back(axismap);
}

} // namespace

ControllerConfig::ControllerConfig(uinpp::MultiDevice& uinput, int slot, bool extra_devices, ControllerOptions const& opts) :
  m_modifier(),
  m_emitter(uinput, slot, extra_devices, opts.uinput)
{
  // Synthesize dpad_x/dpad_y (and trigger axis) from digital pads / LT+RT.
  // Default uinput maps use gamepad.dpad_x/y → ABS_HAT0*; without this,
  // classic Xbox dpad keys never reach the hat axes.
  m_modifier.push_back(ModifierPtr(new CompatModifier));

  add_axis_filters(m_modifier, opts.calibration_map);

  if (opts.deadzone)
  {
    std::shared_ptr<AxismapModifier> axismap(new AxismapModifier);
    char const* axes[] = { "X1", "Y1", "X2", "Y2" };
    for(size_t i = 0; i < sizeof(axes)/sizeof(axes[0]); ++i)
    {
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(-opts.deadzone,
                                                               opts.deadzone,
                                                               true)));
    }
    m_modifier.push_back(axismap);
  }

  if (opts.deadzone_trigger)
  {
    std::shared_ptr<AxismapModifier> axismap(new AxismapModifier);
    char const* axes[] = { "LT", "RT" };
    for(size_t i = 0; i < sizeof(axes)/sizeof(axes[0]); ++i)
    {
      // Triggers are half-axes (0..max); deadzone is a positive threshold from 0.
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(0,
                                                               opts.deadzone_trigger,
                                                               true)));
    }
    m_modifier.push_back(axismap);
  }

  if (opts.square_axis)
  {
    m_modifier.push_back(ModifierPtr(new SquareAxisModifier("X1", "Y1", "X1", "Y1")));
    m_modifier.push_back(ModifierPtr(new SquareAxisModifier("X2", "Y2", "X2", "Y2")));
  }

  add_axis_filters(m_modifier, opts.sensitivity_map);

  if (opts.four_way_restrictor)
  {
    m_modifier.push_back(ModifierPtr(new FourWayRestrictorModifier("X1", "Y1", "X1", "Y1")));
    m_modifier.push_back(ModifierPtr(new FourWayRestrictorModifier("X2", "Y2", "X2", "Y2")));
  }

  add_axis_filters(m_modifier, opts.relative_axis_map);

  if (opts.dpad_rotation)
  {
    m_modifier.push_back(ModifierPtr(new DpadRotationModifier(opts.dpad_rotation)));
  }

  if (!opts.autofire_map.empty())
  {
    std::shared_ptr<ButtonmapModifier> buttonmap(new ButtonmapModifier);
    for(std::map<std::string, ButtonFilterPtr>::const_iterator i = opts.autofire_map.begin();
        i != opts.autofire_map.end(); ++i)
    {
      buttonmap->add_filter(i->first, i->second);
    }
    m_modifier.push_back(buttonmap);
  }

  // axismap / buttonmap last so earlier modifiers still see the original names
  if (!opts.buttonmap.empty())
  {
    m_modifier.push_back(ModifierPtr(ButtonmapModifier::from_option(opts.buttonmap)));
  }

  if (!opts.axismap.empty())
  {
    m_modifier.push_back(ModifierPtr(AxismapModifier::from_option(opts.axismap)));
  }

  for(std::vector<ModifierOption>::const_iterator i = opts.modifier.begin(); i != opts.modifier.end(); ++i)
  {
    m_modifier.push_back(ModifierPtr(Modifier::from_string(i->lhs, i->rhs)));
  }
}

std::vector<ModifierPtr>&
ControllerConfig::get_modifier()
{
  return m_modifier;
}

EventEmitter&
ControllerConfig::get_emitter()
{
  return m_emitter;
}

} // namespace xboxdrv

/* EOF */
