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

#include "virtualkeyboard/status_icon.hpp"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkmain.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"

template<class C, class Call>
void c_callback(GtkMenuItem* menuitem,
                gpointer     userdata)
{
  static_cast<C>(userdata)->Call(menuitem);
}

// http://developer.gimp.org/api/2.0/gdk-pixbuf/gdk-pixbuf-creating.html
// http://developer.gnome.org/gtk/2.24/GtkStatusIcon.html#gtk-status-icon-new-from-pixbuf

StatusIcon::StatusIcon() :
  m_status_icon(),
  m_menu()
{
  /*
    GdkPixbuf* gdk_pixbuf_new_from_data(const guchar *data,
    GdkColorspace colorspace,
    gboolean has_alpha,
    int bits_per_sample,
    int width,
    int height,
    int rowstride,
    GdkPixbufDestroyNotify destroy_fn,
    gpointer destroy_fn_data);
  */
  m_status_icon = gtk_status_icon_new_from_stock(GTK_STOCK_OPEN);
  gtk_status_icon_set_title(m_status_icon, "Virtual Keyboard");
  gtk_status_icon_set_tooltip_text(m_status_icon, "Virtual Keyboard Tooltip");
 
  m_menu = GTK_MENU(gtk_menu_new());
  GtkImageMenuItem *menuitem;
  
  menuitem = GTK_IMAGE_MENU_ITEM(gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL));
  gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "Quit");
  g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(&StatusIcon::on_quit_wrap), this);
  gtk_menu_shell_append(GTK_MENU_SHELL(m_menu), GTK_WIDGET(menuitem));

  g_signal_connect(G_OBJECT(m_status_icon), "popup-menu",
                   G_CALLBACK(&StatusIcon::on_menu_popup_wrap), this);
}

void
StatusIcon::on_quit(GtkMenuItem* menuitem)
{
  gtk_main_quit();
}

void
StatusIcon::on_menu_popup(GtkStatusIcon *status_icon,
                          guint button,
                          guint activate_time)
{
  /*
  void gtk_status_icon_position_menu(GtkMenu *menu,
                                     gint *x,
                                     gint *y,
                                     gboolean *push_in,
                                     gpointer user_data);

  void gtk_menu_popup(GtkMenu *menu,
                      GtkWidget *parent_menu_shell,
                      GtkWidget *parent_menu_item,
                      GtkMenuPositionFunc func,
                      gpointer data,
                      guint button,
                      guint32 activate_time);
  */
    
  gtk_widget_show_all(GTK_WIDGET(m_menu));

  gtk_menu_popup(GTK_MENU(m_menu),
                 NULL,
                 NULL,
                 gtk_status_icon_position_menu,
                 status_icon,
                 button,
                 activate_time);
}

/* EOF */
