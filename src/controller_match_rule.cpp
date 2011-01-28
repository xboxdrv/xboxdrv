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

#include "controller_match_rule.hpp"

#include <assert.h>
#include <boost/tokenizer.hpp>

#include "raise_exception.hpp"

class ControllerMatchRuleProperty : public ControllerMatchRule
{
private:
  std::string m_name;
  std::string m_value;

public:
  ControllerMatchRuleProperty(const std::string& name,
                              const std::string& value) :
    m_name(name),
    m_value(value)
  {}

  bool match(udev_device* device) const
  {
    const char* str = udev_device_get_property_value(device, m_name.c_str());

    log_debug("matching property '" << m_name << "' with value '" << (str?str:"(null)") <<
              "' against '" << m_value);

    if (!str)
    {
      return false;
    }
    else
    {
      return (m_value == str);
    }
  }
};

ControllerMatchRuleGroup::ControllerMatchRuleGroup() :
  m_rules()
{}

void
ControllerMatchRuleGroup::add_rule(ControllerMatchRulePtr rule)
{
  m_rules.push_back(rule);
}

void
ControllerMatchRuleGroup::add_rule_from_string(const std::string& lhs, const std::string& rhs)
{
  m_rules.push_back(ControllerMatchRule::from_string(lhs, rhs));
}

bool
ControllerMatchRuleGroup::match(udev_device* device) const
{
  log_debug("matching group rule");
  for(Rules::const_iterator i = m_rules.begin(); i != m_rules.end(); ++i)
  {
    if (!(*i)->match(device))
    {
      return false;
    }
  }
  return true;
}

bool
ControllerMatchRule::match(udev_device* device) const
{
  assert(!"implement me");
  return false;
#if 0
        const char* serial = udev_device_get_property_value(device, "ID_SERIAL_SHORT");


  switch(m_type)
  {
    case kMatchEverything:
      return true;

    case kMatchUSBId:
      return (vendor == m_vendor && product == m_product);

    case kMatchUSBPath:
      return (bus == m_bus && dev == m_dev);

    case kMatchUSBSerial:
      return serial && (m_serial == serial);

    case kMatchEvdevPath:
      assert(!"not implemented");
      return false;
      
    default:
      assert(!"never reached");
      return false;
  }
#endif
}

ControllerMatchRulePtr
ControllerMatchRule::from_string(const std::string& lhs,
                                 const std::string& rhs)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(rhs, boost::char_separator<char>(":", "", boost::keep_empty_tokens));
  std::vector<std::string> args(tokens.begin(), tokens.end());

  if (lhs == "usbid")
  {
    if (args.size() != 2)
    {
      raise_exception(std::runtime_error, "usbid requires VENDOR:PRODUCT argument");
    }
    else
    {
      boost::shared_ptr<ControllerMatchRuleGroup> group(new ControllerMatchRuleGroup);
                                                    
      group->add_rule(ControllerMatchRulePtr(new ControllerMatchRuleProperty("ID_VENDOR_ID", args[0])));
      group->add_rule(ControllerMatchRulePtr(new ControllerMatchRuleProperty("ID_MODEL_ID", args[1])));
      
      return group;
    }
  }
  else if (lhs == "vendor")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "vendor requires an argument");
    }
    else
    {
      return ControllerMatchRulePtr(new ControllerMatchRuleProperty("ID_VENDOR_ID", args[0]));
    }
  }
  else if (lhs == "property")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "property two arguments");
    }
    else
    {
      return ControllerMatchRulePtr(new ControllerMatchRuleProperty(args[0], args[1]));
    }
  }
  else if (lhs == "product")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "product requires an argument");
    }
    else
    {
      return ControllerMatchRulePtr(new ControllerMatchRuleProperty("ID_MODEL_ID", args[0]));
    }
  }
  else if (lhs == "usbpath")
  {
    if (args.size() != 2)
    {
      raise_exception(std::runtime_error, "usbpath requires BUS:DEV argument");
    }
    else
    {
      boost::shared_ptr<ControllerMatchRuleGroup> group(new ControllerMatchRuleGroup);
                                                    
      group->add_rule(ControllerMatchRulePtr(new ControllerMatchRuleProperty("BUSNUM", args[0])));
      group->add_rule(ControllerMatchRulePtr(new ControllerMatchRuleProperty("DEVNUM", args[1])));
      
      return group;
    }
  }
  else if (lhs == "usbserial")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "usbserial rule requires SERIAL argument");
    }
    else
    {
      return ControllerMatchRulePtr(new ControllerMatchRuleProperty("ID_SERIAL_SHORT", args[0]));
    }
  }
  else if (lhs == "evdev")
  {
    if (args.size() != 1)
    {
      raise_exception(std::runtime_error, "evdev rule requires PATH argument");
    }
    else
    {
      raise_exception(std::runtime_error, "evdev rule not yet implemented");
    }
  }
  else
  {
    raise_exception(std::runtime_error, "'" << lhs << "' not a valid match rule name");
  }
}

/* EOF */
