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

#include "virtualkeyboard/virtual_keyboard.hpp"

#include <iostream>

VirtualKeyboard::VirtualKeyboard()
{
  m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(m_window), 10);

  m_drawing_area = gtk_drawing_area_new();

  gtk_widget_set_size_request(m_drawing_area, 400, 300);

  gtk_container_add(GTK_CONTAINER(m_window), m_drawing_area);

  g_signal_connect(m_drawing_area, "expose-event", 
                   G_CALLBACK(&VirtualKeyboard::on_expose_wrap), this);

  gtk_widget_show(m_drawing_area);
}

VirtualKeyboard::~VirtualKeyboard()
{
  gtk_widget_destroy(m_drawing_area);
  gtk_widget_destroy(m_window);
}

void
VirtualKeyboard::show()
{
  gtk_widget_show(m_window);
}

void
VirtualKeyboard::hide()
{
}



void
VirtualKeyboard::on_expose(GtkWidget* widget, GdkEventExpose* event)
{
  std::cout << "on-expose" << std::endl;

  cairo_t *cr = gdk_cairo_create (widget->window);

  cairo_rectangle(cr, 
                  event->area.x, event->area.y, 
                  event->area.width, event->area.height);
  cairo_clip(cr);

  draw_keyboard(cr);
    
  cairo_destroy(cr);
}

void
VirtualKeyboard::draw_keyboard(cairo_t* cr)
{
  cairo_save(cr);

  cairo_translate(cr, 0, 0);

  int kw = 48;
  int kh = 48;
  cairo_rectangle(cr, 2, 2, kw-4, kh-4);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill(cr);

  cairo_restore(cr);
}

void
VirtualKeyboard::draw_key(cairo_t* cr, int x, int y)
{
  
}

/* EOF */
