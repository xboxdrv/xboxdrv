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

VirtualKeyboard::VirtualKeyboard() :
  m_keyboard(KeyboardDescription::create_us_layout()),
  m_key_width(48),
  m_key_height(48)
{
  m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(m_window), 10);

  m_drawing_area = gtk_drawing_area_new();
  
  GdkColor color;
  gdk_color_parse("#444", &color);
  gtk_widget_modify_bg(m_window, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(m_drawing_area, GTK_STATE_NORMAL, &color);

  gtk_widget_set_size_request(m_window, 1280, 400);
  //gtk_widget_set_size_request(m_drawing_area, 1280, 400);

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
  gtk_widget_hide(m_window);
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
  cairo_select_font_face(cr, "Vera", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

  for(int y = 0; y < m_keyboard.get_height(); ++y)
  {
    for(int x = 0; x < m_keyboard.get_width(); ++x)
    {
      draw_key(cr, x, y, m_keyboard.get_key(x, y));
    }
  }
}

void
VirtualKeyboard::draw_key(cairo_t* cr, int x, int y, const Key& key)
{
  if (key)
  {
    cairo_save(cr);

    cairo_translate(cr, x * m_key_width, y * m_key_height);

    cairo_rectangle(cr, 2, 2, m_key_width * key.m_xspan - 4, m_key_height * key.m_yspan - 4);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    cairo_rectangle(cr, 6, 4, m_key_width * key.m_xspan - 12, m_key_height * key.m_yspan - 12);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);

    cairo_text_extents_t extents;
    cairo_text_extents(cr, key.m_label.c_str(), &extents);

    switch(key.m_style)
    {
      case Key::kLetter:
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr, (m_key_width - extents.width)/2.0, 24);
        cairo_show_text(cr, key.m_label.c_str());
        break;

      case Key::kFunction:
        cairo_set_font_size(cr, 14.0);
        cairo_move_to(cr, 24 - extents.width/2, 24);
        cairo_show_text(cr, key.m_label.c_str());
        break;

      case Key::kModifier:
        cairo_set_font_size(cr, 11.0);
        cairo_move_to(cr, (m_key_width * key.m_xspan)/2.0 - extents.width/2, 24);
        cairo_show_text(cr, key.m_label.c_str());
        break;
    }
 
    cairo_restore(cr);
  }
}

/* EOF */
