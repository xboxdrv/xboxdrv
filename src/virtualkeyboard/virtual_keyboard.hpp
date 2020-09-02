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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_VIRTUAL_KEYBOARD_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_VIRTUAL_KEYBOARD_HPP

#include <functional>
#include <gtk/gtk.h>
#include <string>
#include <vector>

#include "keyboard_description.hpp"

class VirtualKeyboard
{
private:
  KeyboardDescriptionPtr m_keyboard;

  GtkWidget* m_window;
  GtkWidget* m_vbox;
  GtkWidget* m_drawing_area;

  int m_key_width;
  int m_key_height;

  bool m_shift_mode;

  int m_cursor_x;
  int m_cursor_y;

  std::function<void (const Key&, bool)> m_key_callback;

public:
  VirtualKeyboard(KeyboardDescriptionPtr keyboard_desc);
  ~VirtualKeyboard();

  void show();
  void hide();

  int get_width() const;
  int get_height() const;

  void cursor_set(int x, int y);

  void cursor_left();
  void cursor_right();

  void cursor_up();
  void cursor_down();

  void send_key(bool value);

  void move(int x, int y);
  void get_position(int* x, int* y);

  KeyboardDescriptionPtr get_description() const { return m_keyboard; }

  void set_key_callback(const std::function<void (const Key&, bool)>& callback);

  void set_shift_mode(bool shift);

private:
  void on_configure(GtkWindow *window, GdkEvent *event);
  void on_draw(GtkWidget *widget, cairo_t *cr, gpointer userdata);
  void on_key_press(GtkWidget* widget, GdkEventKey* event);
  void on_key_release(GtkWidget* widget, GdkEventKey* event);

  void draw_keyboard(cairo_t* cr);
  void draw_key(cairo_t* cr, int x, int y, const Key& key, bool highlight);
  void draw_centered_text(cairo_t* cr, double x, double y, const std::string& str);

  Key* get_current_key() const;

private:
  static void on_draw_wrap(GtkWidget *widget, cairo_t *cr, gpointer userdata) {
    static_cast<VirtualKeyboard*>(userdata)->on_draw(widget, cr, userdata);
  }

  static void on_key_press_wrap(GtkWidget* widget, GdkEventKey* event, gpointer userdata) {
    static_cast<VirtualKeyboard*>(userdata)->on_key_press(widget, event);
  }

  static void on_key_release_wrap(GtkWidget* widget, GdkEventKey* event, gpointer userdata) {
    static_cast<VirtualKeyboard*>(userdata)->on_key_release(widget, event);
  }

  static void on_configure_wrap(GtkWindow *window, GdkEvent *event, gpointer userdata) {
    static_cast<VirtualKeyboard*>(userdata)->on_configure(window, event);
  }

  static void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
  }

private:
  VirtualKeyboard(const VirtualKeyboard&);
  VirtualKeyboard& operator=(const VirtualKeyboard&);
};

#endif

/* EOF */
