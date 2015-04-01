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

#include "controller_config.hpp"

#include "axisfilter/deadzone_axis_filter.hpp"
#include "controller_options.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

ControllerConfig::ControllerConfig(UInput& uinput, int slot, bool extra_devices, const ControllerOptions& opts) :
  m_modifier(),
  m_emitter(uinput, slot, extra_devices, opts.uinput)
{
  // create modifier
  if (!opts.calibration_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<std::string, AxisFilterPtr>::const_iterator i = opts.calibration_map.begin();
        i != opts.calibration_map.end();
        ++i)
    {
      // BROKEN: axismap->add_filter(i->first, i->second);
    }

    m_modifier.push_back(axismap);
  }

  if (opts.deadzone)
  {
#if 0
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    XboxAxis axes[] = { XBOX_AXIS_X1,
                        XBOX_AXIS_Y1,

                        XBOX_AXIS_X2,
                        XBOX_AXIS_Y2 };

    for(size_t i = 0; i < sizeof(axes)/sizeof(XboxAxis); ++i)
    {
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(-opts.deadzone,
                                                               opts.deadzone,
                                                               true)));
    }

    m_modifier.push_back(axismap);
#endif
  }

  if (opts.deadzone_trigger)
  {
#if 0
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    XboxAxis axes[] = { XBOX_AXIS_LT,
                        XBOX_AXIS_RT };

    for(size_t i = 0; i < sizeof(axes)/sizeof(XboxAxis); ++i)
    {
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(-opts.deadzone_trigger,
                                                               opts.deadzone_trigger,
                                                               true)));
    }

    m_modifier.push_back(axismap);
#endif
  }

  if (opts.square_axis)
  {
    m_modifier.push_back(ModifierPtr(new SquareAxisModifier("X1", "Y1", "X1", "Y1")));
    m_modifier.push_back(ModifierPtr(new SquareAxisModifier("X2", "Y2", "X2", "Y2")));
  }

  if (!opts.sensitivity_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<std::string, AxisFilterPtr>::const_iterator i = opts.sensitivity_map.begin();
        i != opts.sensitivity_map.end(); ++i)
    {
      // BROKEN: axismap->add_filter(i->first, i->second);
    }

    m_modifier.push_back(axismap);
  }

  if (opts.four_way_restrictor)
  {
    m_modifier.push_back(ModifierPtr(new FourWayRestrictorModifier("X1", "Y1", "X1", "Y1")));
    m_modifier.push_back(ModifierPtr(new FourWayRestrictorModifier("X2", "Y2", "X2", "Y2")));
  }

  if (!opts.relative_axis_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<std::string, AxisFilterPtr>::const_iterator i = opts.relative_axis_map.begin();
        i != opts.relative_axis_map.end(); ++i)
    {
      // BROKEN: axismap->add_filter(i->first, i->second);
    }

    m_modifier.push_back(axismap);
  }

  if (opts.dpad_rotation)
  {
    m_modifier.push_back(ModifierPtr(new DpadRotationModifier(opts.dpad_rotation)));
  }

#if 0
  if (!opts.autofire_map.empty())
  {
    boost::shared_ptr<ButtonmapModifier> buttonmap(new ButtonmapModifier);

    for(std::map<std::string, ButtonFilterPtr>::const_iterator i = opts.autofire_map.begin();
        i != opts.autofire_map.end(); ++i)
    {
      buttonmap->add_filter(i->first, i->second);
    }

    m_modifier.push_back(buttonmap);
  }
#endif

  // axismap, buttonmap comes last, as otherwise they would mess up the button and axis names
  for(std::vector<ButtonMappingOption>::const_iterator i = opts.buttonmap.begin(); i != opts.buttonmap.end(); ++i)
  {
    m_modifier.push_back(ModifierPtr(ButtonmapModifier::from_option(opts.buttonmap)));
  }

  for(std::vector<AxisMappingOption>::const_iterator i = opts.axismap.begin(); i != opts.axismap.end(); ++i)
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

/* EOF */
