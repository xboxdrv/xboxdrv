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

#include "key_copy_modifier.hpp"

#include <stdexcept>

#include "evdev_helper.hpp"
#include "raise_exception.hpp"

KeyCopyModifier*
KeyCopyModifier::from_string(const std::vector<std::string>& args, 
                             const ControllerMessageDescriptor& msg_desc)
{
  if (args.size() != 2)
  {
    raise_exception(std::runtime_error, "key-copy needs two arguments");
  }
  else
  {
    return new KeyCopyModifier(str2key(args[0]), str2key(args[1]));
  }
}

KeyCopyModifier::KeyCopyModifier(int from, int to) :
  m_from(from),
  m_to(to)
{
}
  
void
KeyCopyModifier::update(int msec_delta, ControllerMessage& msg)
{
  msg.set_key(m_to, msg.get_key(m_from));
}

std::string
KeyCopyModifier::str() const
{
  std::ostringstream os;
  os << "key-copy:";
  os << key2str(m_from) << ":";
  os << key2str(m_to);
  return os.str();
}

/* EOF */
