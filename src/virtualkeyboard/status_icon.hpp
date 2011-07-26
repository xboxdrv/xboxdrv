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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_STATUS_ICON_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_STATUS_ICON_HPP

#include <gtk/gtkstatusicon.h>
#include <gtk/gtkmenuitem.h>

class StatusIcon
{
private:
  GtkStatusIcon* m_status_icon;
  GtkMenu* m_menu;

public:
  StatusIcon();

private:
  static void on_quit_wrap(GtkMenuItem* menuitem, gpointer userdata) 
  {
    static_cast<StatusIcon*>(userdata)->on_quit(menuitem);
  }
  void on_quit(GtkMenuItem* menuitem);

  static void on_menu_popup_wrap(GtkStatusIcon *status_icon,
                                 guint button,
                                 guint activate_time,
                                 gpointer userdata) 
  {
    static_cast<StatusIcon*>(userdata)->on_menu_popup(status_icon, button, activate_time);
  }

  void on_menu_popup(GtkStatusIcon *status_icon,
                     guint button,
                     guint activate_time);

private:
  StatusIcon(const StatusIcon&);
  StatusIcon& operator=(const StatusIcon&);
};

#endif

/* EOF */
