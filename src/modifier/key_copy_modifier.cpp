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

#include "key_copy_modifier.hpp"

#include <stdexcept>

#include "evdev_helper.hpp"
#include "raise_exception.hpp"

namespace xboxdrv {

KeyCopyModifier*
KeyCopyModifier::from_string(std::vector<std::string> const& args)
{
  if (args.size() != 2)
  {
    raise_exception(std::runtime_error, "key-copy needs two arguments");
  }
  else
  {
    return new KeyCopyModifier(args[0], args[1]);
  }
}

KeyCopyModifier::KeyCopyModifier(std::string const& from, std::string const& to) :
  m_from(from),
  m_to(to),
  m_from_sym(-1),
  m_to_sym(-1)
{
}

void
KeyCopyModifier::init(ControllerMessageDescriptor& desc)
{
  m_from_sym = desc.key().put(m_from);
  m_to_sym   = desc.key().getput(m_to);
}

void
KeyCopyModifier::update(int msec_delta, ControllerMessage& msg, ControllerMessageDescriptor const& desc)
{
  msg.set_key(m_to_sym, msg.get_key(m_from_sym));
}

std::string
KeyCopyModifier::str() const
{
  std::ostringstream os;
  os << "key-copy:";
  os << m_from << ":";
  os << m_to;
  return os.str();
}

} // namespace xboxdrv

/* EOF */
