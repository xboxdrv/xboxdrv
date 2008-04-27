/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include "xbox360_driver.hpp"
#include "uinput_driver.hpp"
#include "abs_to_rel.hpp"
#include "toggle_button.hpp"
#include "autofire_button.hpp"
#include "join_axis.hpp"
#include "btn_to_abs.hpp"
#include "file_reader.hpp"
#include "control_factory.hpp"

ControlFactory::ControlFactory()
{

}

Control*
ControlFactory::create(const std::string& name, FileReader reader)
{
  if (name == "join-abs")
    {
      return new JoinAxis();
    }
  else if (name == "btn-to-abs")
    {
      return new BtnToAbs();
    }
  else if (name == "abs-to-btn")
    {
      int threshold = 0;
      reader.get("threshold", threshold);      
      return new AbsToBtn(threshold);
    }
  else
    {
      LOG_ERROR("ControlFactory: unknown control: '" << name << "'");
      return 0;
    }
}

/* EOF */
