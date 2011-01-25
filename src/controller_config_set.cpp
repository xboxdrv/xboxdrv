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

#include "controller_config_set.hpp"

#include "log.hpp"

#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

ControllerConfigSetPtr
ControllerConfigSet::create(UInput& uinput, int slot, bool extra_devices, const Options::ControllerConfigs& opts)
{  
  ControllerConfigSetPtr m_config(new ControllerConfigSet);

  for(Options::ControllerConfigs::const_iterator i = opts.begin();
      i != opts.end(); ++i)
  {
    const ControllerOptions& ctrl_opt = i->second;

    ControllerConfigPtr config(new ControllerConfig(uinput, slot, extra_devices, ctrl_opt));
    create_modifier(ctrl_opt, &config->get_modifier());
    m_config->add_config(config);

#ifdef FIXME
    // introspection of the config
    std::cout << "==[[ Active Modifier ]]==" << std::endl;
    for(std::vector<ModifierPtr>::iterator mod = config->get_modifier().begin(); 
        mod != config->get_modifier().end(); 
        ++mod)
    {
      std::cout << (*mod)->str() << std::endl;
    }
#endif
  }
  
  log_info << "UInput finish" << std::endl;

  return m_config;
}

void
ControllerConfigSet::create_modifier(const ControllerOptions& opts, std::vector<ModifierPtr>* modifier)
{
  if (!opts.calibration_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.calibration_map.begin();
        i != opts.calibration_map.end();
        ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.deadzone)
  {
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

    modifier->push_back(axismap);
  }

  if (opts.deadzone_trigger)
  {
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

    modifier->push_back(axismap);
  }

  if (opts.square_axis)
  {
    modifier->push_back(ModifierPtr(new SquareAxisModifier(XBOX_AXIS_X1, XBOX_AXIS_Y1)));
    modifier->push_back(ModifierPtr(new SquareAxisModifier(XBOX_AXIS_X2, XBOX_AXIS_Y2)));
  }

  if (!opts.sensitivity_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.sensitivity_map.begin();
        i != opts.sensitivity_map.end(); ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.four_way_restrictor)
  {
    modifier->push_back(ModifierPtr(new FourWayRestrictorModifier(XBOX_AXIS_X1, XBOX_AXIS_Y1)));
    modifier->push_back(ModifierPtr(new FourWayRestrictorModifier(XBOX_AXIS_X2, XBOX_AXIS_Y2)));
  }

  if (!opts.relative_axis_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.relative_axis_map.begin();
        i != opts.relative_axis_map.end(); ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.dpad_rotation)
  {
    modifier->push_back(ModifierPtr(new DpadRotationModifier(opts.dpad_rotation)));
  }

  if (!opts.autofire_map.empty())
  {
    boost::shared_ptr<ButtonmapModifier> buttonmap(new ButtonmapModifier);

    for(std::map<XboxButton, ButtonFilterPtr>::const_iterator i = opts.autofire_map.begin();
        i != opts.autofire_map.end(); ++i)
    {
      buttonmap->add_filter(i->first, i->second); 
    }

    modifier->push_back(buttonmap);
  }

  // axismap, buttonmap comes last, as otherwise they would mess up the button and axis names
  if (!opts.buttonmap->empty())
  {
    modifier->push_back(opts.buttonmap);
  }

  if (!opts.axismap->empty())
  {
    modifier->push_back(opts.axismap);
  }

  modifier->insert(modifier->end(), opts.modifier.begin(), opts.modifier.end());
}

ControllerConfigSet::ControllerConfigSet() :
  m_config(),
  m_current_config(0)
{
}

void
ControllerConfigSet::next_config()
{
  m_current_config += 1;

  if (m_current_config >= static_cast<int>(m_config.size()))
  {
    m_current_config = 0;
  }
}

void
ControllerConfigSet::prev_config()
{
  m_current_config -= 1;
  
  if (m_current_config < 0)
  {
    m_current_config = static_cast<int>(m_config.size()) - 1;
  }
}

int
ControllerConfigSet::config_count() const
{
  return static_cast<int>(m_config.size());
}

ControllerConfigPtr
ControllerConfigSet::get_config(int i) const
{
  assert(i >= 0);
  assert(i < static_cast<int>(m_config.size()));

  return m_config[i];
}

ControllerConfigPtr
ControllerConfigSet::get_config() const
{
  assert(!m_config.empty());

  return m_config[m_current_config];
}

void
ControllerConfigSet::add_config(ControllerConfigPtr config)
{
  m_config.push_back(config);
}

/* EOF */
