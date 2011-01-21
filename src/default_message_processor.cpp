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

#include "log.hpp"
#include "options.hpp"
#include "uinput.hpp"

#include "modifier/axismap_modifier.hpp"
#include "modifier/buttonmap_modifier.hpp"
#include "modifier/dpad_rotation_modifier.hpp"
#include "modifier/four_way_restrictor_modifier.hpp"
#include "modifier/square_axis_modifier.hpp"

DefaultMessageProcessor::DefaultMessageProcessor(uInput& uinput, const Options& opts) :
  m_uinput(uinput),
  m_config_toggle_button(opts.config_toggle_button)
{
  memset(&m_oldmsg, 0, sizeof(m_oldmsg));

  // create ControllerConfigs
  for(std::vector<ControllerOptions>::const_iterator i = opts.controller.begin();
      i != opts.controller.end(); 
      ++i)
  {
    ControllerConfigPtr config(new ControllerConfig(uinput, *i));
    create_modifier(*i, &config->get_modifier());
    m_config.add_config(config);

    // introspection of the config
    std::cout << "==[[ Active Modifier ]]==" << std::endl;
    for(std::vector<ModifierPtr>::iterator mod = config->get_modifier().begin(); 
        mod != config->get_modifier().end(); 
        ++mod)
    {
      std::cout << (*mod)->str() << std::endl;
    }
  }

  log_info << "UInput finish" << std::endl;

  // After all the ControllerConfig registered their events, finish up
  // the device creation
  uinput.finish();
}

DefaultMessageProcessor::~DefaultMessageProcessor()
{
}

void
DefaultMessageProcessor::create_modifier(const ControllerOptions& opts, std::vector<ModifierPtr>* modifier)
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

void
DefaultMessageProcessor::send(const XboxGenericMsg& msg_in, int msec_delta)
{
  if (!m_config.empty())
  {
    XboxGenericMsg msg = msg_in; 

    // handling switching of configurations
    if (m_config_toggle_button != XBOX_BTN_UNKNOWN)
    {
      bool last = get_button(m_oldmsg, m_config_toggle_button);
      bool cur  = get_button(msg, m_config_toggle_button);

      if (cur && cur != last)
      {
        // reset old mapping to zero to not get stuck keys/axis
        m_config.get_config()->get_uinput().reset_all_outputs();

        // switch to the next input mapping
        m_config.next_config();

        log_info << "Next Config" << std::endl;
      }
    }

    // run the controller message through all modifier
    for(std::vector<ModifierPtr>::iterator i = m_config.get_config()->get_modifier().begin();
        i != m_config.get_config()->get_modifier().end(); 
        ++i)
    {
      (*i)->update(msec_delta, msg);
    }

    m_config.get_config()->get_uinput().update(msec_delta);

    // send current Xbox state to uinput
    if (memcmp(&msg, &m_oldmsg, sizeof(XboxGenericMsg)) != 0)
    {
      // Only send a new event out if something has changed,
      // this is useful since some controllers send events
      // even if nothing has changed, deadzone can cause this
      // too
      m_oldmsg = msg;

      m_config.get_config()->get_uinput().send(msg);
    }
  }
}

/* EOF */
