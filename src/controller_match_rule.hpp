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

#ifndef HEADER_XBOXDRV_CONTROLLER_MATCH_RULE_HPP
#define HEADER_XBOXDRV_CONTROLLER_MATCH_RULE_HPP

#include <boost/shared_ptr.hpp>
extern "C" {
#include <libudev.h>
}
#include <string>
#include <vector>

struct udev_device;
class ControllerMatchRule;
typedef boost::shared_ptr<ControllerMatchRule> ControllerMatchRulePtr;

class ControllerMatchRule
{
public:
  static ControllerMatchRulePtr from_string(const std::string& lhs,
                                            const std::string& rhs);

public:
  ControllerMatchRule() {}
  virtual ~ControllerMatchRule() {}

  virtual bool match(udev_device* device) const =0;
};

class ControllerMatchRuleGroup : public ControllerMatchRule
{
private:
  typedef std::vector<ControllerMatchRulePtr> Rules;
  Rules m_rules;

public:
  ControllerMatchRuleGroup();

  void add_rule(ControllerMatchRulePtr rule);
  void add_rule_from_string(const std::string& lhs, const std::string& rhs);
  bool match(udev_device* device) const;
};

#endif

/* EOF */
