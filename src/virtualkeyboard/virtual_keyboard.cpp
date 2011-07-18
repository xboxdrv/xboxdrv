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

#include <gdk/gdkkeysyms.h>
#include <iostream>

#pragma GCC diagnostic ignored "-Wold-style-cast"

VirtualKeyboard::VirtualKeyboard(const KeyboardDescription& keyboard_desc) :
  m_keyboard(keyboard_desc),
  m_window(0),
  m_drawing_area(0),
  m_key_width(48),
  m_key_height(48),
  m_shift_mode(false),
  m_cursor_x(0),
  m_cursor_y(0),
  m_key_callback()
{
  m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(m_window), 10);

  m_drawing_area = gtk_drawing_area_new();
  
  GdkColor color;
  gdk_color_parse("#222", &color);
  gtk_widget_modify_bg(m_window, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(m_drawing_area, GTK_STATE_NORMAL, &color);

  gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
  //gtk_window_set_accept_focus(GTK_WINDOW(m_window), FALSE);

  //  gtk_widget_set_size_request(m_window, 1280, 400);
  gtk_widget_set_size_request(m_drawing_area, get_width(), get_height());

  gtk_container_add(GTK_CONTAINER(m_window), m_drawing_area);

  g_signal_connect(m_drawing_area, "expose-event", 
                   G_CALLBACK(&VirtualKeyboard::on_expose_wrap), this);

  g_signal_connect(m_window, "key-press-event", 
                   G_CALLBACK(&VirtualKeyboard::on_key_press_wrap), this);
  g_signal_connect(m_window, "key-release-event", 
                   G_CALLBACK(&VirtualKeyboard::on_key_release_wrap), this);

  gtk_widget_show(m_drawing_area);
}

VirtualKeyboard::~VirtualKeyboard()
{
  gtk_widget_destroy(m_drawing_area);
  gtk_widget_destroy(m_window);
}

int
VirtualKeyboard::get_width() const
{
  return m_key_width * m_keyboard.get_width() + 24;
}

int
VirtualKeyboard::get_height() const
{
  return m_key_height * m_keyboard.get_height() + 12;
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
VirtualKeyboard::cursor_set(int x, int y)
{
  m_cursor_x = x;
  m_cursor_y = y;
}

void
VirtualKeyboard::cursor_left()
{
  m_cursor_x -= 1;
}

void
VirtualKeyboard::cursor_right()
{
  m_cursor_x += 1;
}

void
VirtualKeyboard::cursor_up()
{
  m_cursor_y -= 1;
}

void
VirtualKeyboard::cursor_down()
{
  m_cursor_y += 1;
}

void
VirtualKeyboard::on_key_release(GtkWidget* widget, GdkEventKey* event)
{
}

void
VirtualKeyboard::on_key_press(GtkWidget* widget, GdkEventKey* event)
{
  switch (event->keyval)
  {
    case GDK_KEY_Escape: 
      break;

    case GDK_KEY_Left: 
      cursor_left();
      break;

    case GDK_KEY_Right: 
      cursor_right();
      break;

    case GDK_KEY_Up: 
      cursor_up();
      break;

    case GDK_KEY_Down: 
      cursor_down();
      break;

    case GDK_KEY_Return:
      if (m_key_callback)
      {
        m_key_callback(m_keyboard.get_key(m_cursor_x, m_cursor_y), true);
      }
      break;

    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
      m_shift_mode = !m_shift_mode;
      break;
  }

  gtk_widget_queue_draw(m_drawing_area);
}

void
VirtualKeyboard::on_expose(GtkWidget* widget, GdkEventExpose* event)
{
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
      draw_key(cr, x, y, m_keyboard.get_key(x, y), (m_cursor_x == x && m_cursor_y == y));
    }
  }
}

void
VirtualKeyboard::draw_centered_text(cairo_t* cr, double x, double y, const std::string& str)
{
  cairo_font_extents_t font_extents;
  cairo_font_extents(cr, &font_extents);

  cairo_text_extents_t extents;
  cairo_text_extents(cr, str.c_str(), &extents);

  cairo_move_to(cr, 
                (m_key_width - extents.width)/2.0 - extents.x_bearing, 
                (m_key_height + (font_extents.descent + font_extents.ascent * 0.1f))/2.0);
  cairo_show_text(cr, str.c_str()); 
}

void
VirtualKeyboard::draw_key(cairo_t* cr, int x, int y, const Key& key, bool highlight)
{
  if (key)
  {
    cairo_save(cr);

    cairo_translate(cr, x * m_key_width, y * m_key_height);

    { // add some spacing between F'keys, cursor keys and numpad
      int spacing = 12;

      if (y > 0)
        cairo_translate(cr, 0, spacing);

      if (x > 13)
        cairo_translate(cr, spacing, 0);

      if (x > 16)
        cairo_translate(cr, spacing, 0);
    }

    cairo_rectangle(cr, 2, 2, m_key_width * key.m_xspan - 4, m_key_height * key.m_yspan - 4);
    if (highlight)
      cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    else
      cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    cairo_rectangle(cr, 6, 4, m_key_width * key.m_xspan - 12, m_key_height * key.m_yspan - 12);
    if (highlight)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    else
      cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);

    std::string text;

    if (m_shift_mode)
    {
      text = key.m_label;
    }
    else
    {
      if (key.m_shift_label.empty())
        text = key.m_label;
      else
        text = key.m_shift_label;
    }

    switch(key.m_style)
    {
      case Key::kLetter:
        {
          cairo_set_font_size(cr, 18.0);
          draw_centered_text(cr, m_key_width/2.0, m_key_width/2.0, text.c_str());
        }
        break;

      case Key::kFunction:
        {
          cairo_set_font_size(cr, 14.0);
          draw_centered_text(cr, m_key_width/2.0, m_key_width/2.0, text.c_str());
        }
        break;

      case Key::kModifier:
        {
          cairo_set_font_size(cr, 11.0);
          draw_centered_text(cr, m_key_width/2.0, m_key_width/2.0, text.c_str());
        }
        break;
    }
 
    cairo_restore(cr);
  }
}

void
VirtualKeyboard::set_key_callback(const boost::function<void (const Key&, bool)>& callback)
{
  m_key_callback = callback;
}

/* EOF */
