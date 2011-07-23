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

namespace {

int overflow(int pos, int width)
{
  if (pos >= width)
  {
    return pos % width;
  }
  else if (pos < 0)
  {
    return pos % width + width;
  }
  else
  {
    return pos;
  }
}

int advance(int pos, int width, int steps)
{
  pos += steps;
  return overflow(pos, width);
}

} // namespace

VirtualKeyboard::VirtualKeyboard(KeyboardDescriptionPtr keyboard_desc) :
  m_keyboard(keyboard_desc),
  m_window(0),
  m_vbox(0),
  m_drawing_area(0),
  m_key_width(48),
  m_key_height(48),
  m_shift_mode(false),
  m_cursor_x(0),
  m_cursor_y(0),
  m_key_callback()
{
  m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(m_window), 0);

  m_vbox = gtk_vbox_new(FALSE, 0);
  m_drawing_area = gtk_drawing_area_new();
  
  GdkColor color;
  gdk_color_parse("#222", &color);
  gtk_widget_modify_bg(m_window, GTK_STATE_NORMAL, &color);
  gtk_widget_modify_bg(m_drawing_area, GTK_STATE_NORMAL, &color);

  //gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
  gtk_window_set_accept_focus(GTK_WINDOW(m_window), FALSE);

  //gdk_window_set_override_redirect(GTK_WINDOW(m_window)->window, TRUE);
  //gdk_window_set_decorations(GTK_WINDOW(m_window)->window, 0);
  gtk_window_set_decorated(GTK_WINDOW(m_window), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW(m_window), TRUE);

  gtk_window_set_default_size(GTK_WINDOW(m_window), 3*get_width()/4, 3*get_height()/4);
  //gtk_widget_set_size_request(m_window, get_width(), get_height());
  //gtk_widget_set_size_request(m_drawing_area, get_width(), get_height());

  gtk_container_add(GTK_CONTAINER(m_window), GTK_WIDGET(m_vbox));
  gtk_box_pack_start(GTK_BOX(m_vbox), GTK_WIDGET(m_drawing_area), TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(m_window), "destroy", G_CALLBACK(&VirtualKeyboard::on_destroy), NULL);
  //g_signal_connect(G_OBJECT(m_window), "configure-event", G_CALLBACK(&VirtualKeyboard::on_configure_wrap), this);
  g_signal_connect(G_OBJECT(m_drawing_area), "expose-event", 
                   G_CALLBACK(&VirtualKeyboard::on_expose_wrap), this);

  if (true)
  {
    g_signal_connect(m_window, "key-press-event", 
                     G_CALLBACK(&VirtualKeyboard::on_key_press_wrap), this);
    g_signal_connect(m_window, "key-release-event", 
                     G_CALLBACK(&VirtualKeyboard::on_key_release_wrap), this);
  }

  gtk_widget_show(m_drawing_area);
  gtk_widget_show(m_vbox);
}

VirtualKeyboard::~VirtualKeyboard()
{
}

int
VirtualKeyboard::get_width() const
{
  return m_key_width * m_keyboard->get_width() + 24;
}

int
VirtualKeyboard::get_height() const
{
  return m_key_height * m_keyboard->get_height() + 12;
}

void
VirtualKeyboard::show()
{
  /*
    GdkScreen* ;
  GdkWindow* 
  GdkWindow* gdk_window_get_pointer(gdk_screen_get_root_window(gdk_screen_get_default()),
                                    gint *x, gint *y,
                                    GdkModifierType *mask);
  */
  gint x, y;
  GdkModifierType mask;
  gdk_window_get_pointer(gdk_screen_get_root_window(gdk_screen_get_default()),
                         &x, &y, &mask);
  move(x - get_width()/2, y + get_height()/2);
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

  gtk_widget_queue_draw(m_drawing_area);
}

Key*
VirtualKeyboard::get_current_key() const
{
  return m_keyboard->get_key(m_cursor_x, m_cursor_y);
}

void
VirtualKeyboard::cursor_left()
{
  Key* old_key = get_current_key();

  m_cursor_x = advance(m_cursor_x, m_keyboard->get_width(), -1);

  if (old_key)
  {
    old_key = old_key->get_parent();
  }
  if (!m_keyboard->get_key(m_cursor_x, m_cursor_y) ||
      m_keyboard->get_key(m_cursor_x, m_cursor_y)->get_parent() == old_key)
  {
    cursor_left();
  }
  else
  {
    gtk_widget_queue_draw(m_drawing_area);
  }
}

void
VirtualKeyboard::cursor_right()
{
  Key* old_key = get_current_key();

  m_cursor_x = advance(m_cursor_x, m_keyboard->get_width(), 1);

  if (old_key)
  {
    old_key = old_key->get_parent();
  }
  if (!m_keyboard->get_key(m_cursor_x, m_cursor_y) ||
      m_keyboard->get_key(m_cursor_x, m_cursor_y)->get_parent() == old_key)
  {
    cursor_right();
  }
  else
  {
    gtk_widget_queue_draw(m_drawing_area);
  }
}

void
VirtualKeyboard::cursor_up()
{
  Key* old_key = get_current_key();

  m_cursor_y = advance(m_cursor_y, m_keyboard->get_height(), -1);

  if (old_key)
  {
    old_key = old_key->get_parent();
  }
  if (!m_keyboard->get_key(m_cursor_x, m_cursor_y) ||
      m_keyboard->get_key(m_cursor_x, m_cursor_y)->get_parent() == old_key)
  {
    cursor_up();
  }
  else
  {
    gtk_widget_queue_draw(m_drawing_area);
  }
}

void
VirtualKeyboard::cursor_down()
{
  Key* old_key = get_current_key();

  m_cursor_y = advance(m_cursor_y, m_keyboard->get_height(), 1);

  if (old_key)
  {
    old_key = old_key->get_parent();
  }
  if (!m_keyboard->get_key(m_cursor_x, m_cursor_y) ||
      m_keyboard->get_key(m_cursor_x, m_cursor_y)->get_parent() == old_key)
  {
    cursor_down();
  }
  else
  {
    gtk_widget_queue_draw(m_drawing_area);
  }
}

void
VirtualKeyboard::on_key_release(GtkWidget* widget, GdkEventKey* event)
{
  switch (event->keyval)
  {
    case GDK_KEY_Return:
      send_key(false);
      break;
  }
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
      send_key(true);
      break;

    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
      m_shift_mode = !m_shift_mode;
      gtk_widget_queue_draw(m_drawing_area);
      break;
  }
}

void
VirtualKeyboard::send_key(bool value)
{
  if (m_key_callback)
  {
    Key* key = get_current_key();
    if (key)
    {
      m_key_callback(*key, value);
    }
  }
}

void
VirtualKeyboard::move(int x, int y)
{
  gtk_window_move(GTK_WINDOW(m_window), x, y);
}

void
VirtualKeyboard::get_position(int* x, int* y)
{
  gtk_window_get_position(GTK_WINDOW(m_window), x, y);
}

void
VirtualKeyboard::on_configure(GtkWindow *window, GdkEvent *event)
{
  std::cout << event->configure.x << " "
            << event->configure.y << " "
            << event->configure.width << " "
            << event->configure.height
            << std::endl;
}

void
VirtualKeyboard::on_expose(GtkWidget* widget, GdkEventExpose* event)
{
  if (false)
  {
    std::cout << "Size: " << 
      widget->allocation.width << " " << 
      widget->allocation.height << std::endl;
  }

  cairo_t *cr = gdk_cairo_create (widget->window);

  cairo_rectangle(cr, 
                  event->area.x, event->area.y, 
                  event->area.width, event->area.height);
  cairo_clip(cr);

  // scale the keyboard to the size of the window
  cairo_scale(cr,
              static_cast<double>(widget->allocation.width)  / get_width(),
              static_cast<double>(widget->allocation.height) / get_height());

  draw_keyboard(cr);
    
  cairo_destroy(cr);
}

void
VirtualKeyboard::draw_keyboard(cairo_t* cr)
{
  cairo_select_font_face(cr, "Vera", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

  Key* current_key = get_current_key();
  assert(current_key);
  
  for(int y = 0; y < m_keyboard->get_height(); ++y)
  {
    for(int x = 0; x < m_keyboard->get_width(); ++x)
    {
      Key* key = m_keyboard->get_key(x, y);
      if (key && !key->is_ref_key())
      {
        draw_key(cr, x, y, *key, (current_key->get_parent() == key));
      }
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
                x - (extents.width)/2.0 - extents.x_bearing, 
                y + ((font_extents.descent + font_extents.ascent * 0.1f))/2.0);
  cairo_show_text(cr, str.c_str()); 
}

void
VirtualKeyboard::draw_key(cairo_t* cr, int x, int y, const Key& key, bool highlight)
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

  cairo_rectangle(cr, 2, 2, m_key_width * key.get_xspan() - 4, m_key_height * key.get_yspan() - 4);
  if (highlight)
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  else
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  cairo_fill(cr);

  cairo_rectangle(cr, 6, 4, m_key_width * key.get_xspan() - 12, m_key_height * key.get_yspan() - 12);
  if (highlight)
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  else
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0, 0, 0);

  std::string text;

  if (!m_shift_mode)
  {
    text = key.get_label();
  }
  else
  {
    if (key.get_shift_label().empty())
      text = key.get_label();
    else
      text = key.get_shift_label();
  }

  switch(key.get_style())
  {
    case Key::kLetter:
      {
        cairo_set_font_size(cr, 24.0);
        draw_centered_text(cr, (m_key_width * key.get_xspan())/2.0, m_key_height/2.0, text.c_str());
      }
      break;

    case Key::kFunction:
      {
        cairo_set_font_size(cr, 18.0);
        draw_centered_text(cr, (m_key_width * key.get_xspan())/2.0, m_key_height/2.0, text.c_str());
      }
      break;

    case Key::kModifier:
      {
        cairo_set_font_size(cr, 12.0);
        draw_centered_text(cr, (m_key_width * key.get_xspan())/2.0, m_key_height/2.0, text.c_str());
      }
      break;
  }
 
  cairo_restore(cr);
}

void
VirtualKeyboard::set_key_callback(const boost::function<void (const Key&, bool)>& callback)
{
  m_key_callback = callback;
}

/* EOF */
