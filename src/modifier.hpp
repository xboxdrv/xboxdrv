/* 
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_MODIFIER_HPP
#define HEADER_MODIFIER_HPP

#include <boost/shared_ptr.hpp>

#include "controller_message.hpp"
#include "controller_message_descriptor.hpp"

class Modifier;
class Options;

typedef boost::shared_ptr<Modifier> ModifierPtr;

struct ModifierOption
{
  ModifierOption(const std::string& lhs_,
                 const std::string& rhs_) :
    lhs(lhs_),
    rhs(rhs_)
  {}

  std::string lhs;
  std::string rhs;
};

class Modifier
{
public:
  static Modifier* from_string(const std::string& name, const std::string& value,
                               const ControllerMessageDescriptor& msg_desc);

public:
  virtual ~Modifier() {}
  virtual void update(int msec_delta, ControllerMessage& msg) = 0;

  virtual std::string str() const = 0;
};

#endif

/* EOF */
