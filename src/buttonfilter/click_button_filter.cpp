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

#include "buttonfilter/click_button_filter.hpp"

#include <assert.h>

namespace xboxdrv {

ClickButtonFilter::ClickButtonFilter(Mode mode) :
  m_mode(mode),
  m_last_value(false)
{
}

bool
ClickButtonFilter::filter(bool value)
{
  if (m_last_value != value)
  {
    m_last_value = value;

    switch(m_mode)
    {
      case kPress:
        if (value)
        {
          return true;
        }
        else
        {
          return false;
        }
        break;

      case kRelease:
        if (!value)
        {
          return true;
        }
        else
        {
          return false;
        }
        break;

      case kBoth:
        return true;
        break;

      default:
        assert(false && "never reached");
        return false;
    }
  }
  else
  {
    return false;
  }
}

std::string
ClickButtonFilter::str() const
{
  switch(m_mode)
  {
    case kPress:
      return "press";

    case kRelease:
      return "release";

    case kBoth:
      return "both";

    default:
      assert(false && "never reached");
      return {};
  }
}

} // namespace xboxdrv

/* EOF */
