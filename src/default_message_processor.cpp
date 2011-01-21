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

#include "default_message_processor.hpp"

#include <string.h>
#include <iostream>

#include "options.hpp"

#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

DefaultMessageProcessor::DefaultMessageProcessor(uInput& uinput, const Options& opts) :
  m_uinput(uinput)
{
  memset(&m_oldmsg, 0, sizeof(m_oldmsg));

  // Create ControllerConfig
  ControllerConfigPtr config(new ControllerConfig(uinput));
  m_config.add_config(config);

  create_modifier(opts, &config->get_modifier());
  
  // introspection of the config
  std::cout << "Active Modifier:" << std::endl;
  for(std::vector<ModifierPtr>::iterator i = config->get_modifier().begin(); 
      i != config->get_modifier().end(); 
      ++i)
  {
    std::cout << (*i)->str() << std::endl;
  }
}

DefaultMessageProcessor::~DefaultMessageProcessor()
{
}

void
DefaultMessageProcessor::create_modifier(const Options& opts, std::vector<ModifierPtr>* modifier)
{
  if (!opts.controller.back().calibration_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.controller.back().calibration_map.begin();
        i != opts.controller.back().calibration_map.end();
        ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.controller.back().deadzone)
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    XboxAxis axes[] = { XBOX_AXIS_X1,
                        XBOX_AXIS_Y1,
                      
                        XBOX_AXIS_X2,
                        XBOX_AXIS_Y2 };

    for(size_t i = 0; i < sizeof(axes)/sizeof(XboxAxis); ++i)
    {
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(-opts.controller.back().deadzone,
                                                               opts.controller.back().deadzone,
                                                               true)));
    }

    modifier->push_back(axismap);
  }

  if (opts.controller.back().deadzone_trigger)
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    XboxAxis axes[] = { XBOX_AXIS_LT,
                        XBOX_AXIS_RT };

    for(size_t i = 0; i < sizeof(axes)/sizeof(XboxAxis); ++i)
    {
      axismap->add_filter(axes[i],
                          AxisFilterPtr(new DeadzoneAxisFilter(-opts.controller.back().deadzone_trigger,
                                                               opts.controller.back().deadzone_trigger,
                                                               true)));
    }

    modifier->push_back(axismap);
  }

  if (opts.controller.back().square_axis)
  {
    modifier->push_back(ModifierPtr(new SquareAxisModifier(XBOX_AXIS_X1, XBOX_AXIS_Y1)));
    modifier->push_back(ModifierPtr(new SquareAxisModifier(XBOX_AXIS_X2, XBOX_AXIS_Y2)));
  }

  if (!opts.controller.back().sensitivity_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.controller.back().sensitivity_map.begin();
        i != opts.controller.back().sensitivity_map.end(); ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.controller.back().four_way_restrictor)
  {
    modifier->push_back(ModifierPtr(new FourWayRestrictorModifier(XBOX_AXIS_X1, XBOX_AXIS_Y1)));
    modifier->push_back(ModifierPtr(new FourWayRestrictorModifier(XBOX_AXIS_X2, XBOX_AXIS_Y2)));
  }

  if (!opts.controller.back().relative_axis_map.empty())
  {
    boost::shared_ptr<AxismapModifier> axismap(new AxismapModifier);

    for(std::map<XboxAxis, AxisFilterPtr>::const_iterator i = opts.controller.back().relative_axis_map.begin();
        i != opts.controller.back().relative_axis_map.end(); ++i)
    {
      axismap->add_filter(i->first, i->second); 
    }

    modifier->push_back(axismap);
  }

  if (opts.controller.back().dpad_rotation)
  {
    modifier->push_back(ModifierPtr(new DpadRotationModifier(opts.controller.back().dpad_rotation)));
  }

  if (!opts.controller.back().autofire_map.empty())
  {
    boost::shared_ptr<ButtonmapModifier> buttonmap(new ButtonmapModifier);

    for(std::map<XboxButton, ButtonFilterPtr>::const_iterator i = opts.controller.back().autofire_map.begin();
        i != opts.controller.back().autofire_map.end(); ++i)
    {
      buttonmap->add_filter(i->first, i->second); 
    }

    modifier->push_back(buttonmap);
  }

  // axismap, buttonmap comes last, as otherwise they would mess up the button and axis names
  if (!opts.controller.back().buttonmap->empty())
  {
    modifier->push_back(opts.controller.back().buttonmap);
  }

  if (!opts.controller.back().axismap->empty())
  {
    modifier->push_back(opts.controller.back().axismap);
  }

  modifier->insert(modifier->end(), opts.controller.back().modifier.begin(), opts.controller.back().modifier.end());
}

void
DefaultMessageProcessor::send(XboxGenericMsg& msg, int msec_delta)
{
  if (memcmp(&msg, &m_oldmsg, sizeof(XboxGenericMsg)) != 0)
  {
    // Only send a new event out if something has changed,
    // this is useful since some controllers send events
    // even if nothing has changed, deadzone can cause this
    // too
    m_oldmsg = msg;

    if (!m_config.empty())
    {
      // run the controller message through all modifier
      for(std::vector<ModifierPtr>::iterator i = m_config.get_config()->get_modifier().begin();
          i != m_config.get_config()->get_modifier().end(); 
          ++i)
      {
        (*i)->update(msec_delta, msg);
      }

      m_config.get_config()->get_uinput().update(msec_delta);
    }

    // send current Xbox state to uinput
    if (!m_config.empty())
    {
      m_config.get_config()->get_uinput().send(msg);
    }
  }
}

/* EOF */
