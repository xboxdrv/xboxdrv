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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_VIRTUAL_KEYBOARD_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_VIRTUAL_KEYBOARD_HPP

#include <gtk/gtk.h>
#include <string>
#include <vector>

class Key
{
  enum Style { kCenter }; 
    
  //UIEventSequence m_sequence;
  int m_code;
  std::string m_label;
  std::string m_shift_label;
  std::string m_alt_label;
  Style m_style;

  Key(//const UIEventSequence& sequence,
    int code,
      Style style,
      const std::string& label, 
      const std::string& shift_label  = std::string(),
      const std::string& alt_label = std::string()) :
    //m_sequence(sequence),
    m_code(code),
    m_style(style),
    m_label(label),
    m_shift_label(shift_label),
    m_alt_label(alt_label)    
  {}
};

class Keyboard
{
private:
  int m_width;
  int m_height;

  std::vector<Key*> m_keys;

public:
  Keyboard() {}

  int get_width() const  { return m_width; }
  int get_height() const { return m_height; }
  Key* get_key(int x, int y) const { return m_keys[m_width * y + x]; }
};

class VirtualKeyboard
{
private:
  GtkWidget* m_window;
  GtkWidget* m_drawing_area;

  Keyboard m_keyboard;

public:
  VirtualKeyboard();
  ~VirtualKeyboard();

  void show();
  void hide();

  void on_expose(GtkWidget* widget, GdkEventExpose* event);

  void draw_keyboard(cairo_t* cr);
  void draw_key(cairo_t* cr, int x, int y);

private:
  Key* get_key(int x, int y);

private:
  static void on_expose_wrap(GtkWidget* widget, GdkEventExpose* event, gpointer userdata) {
    static_cast<VirtualKeyboard*>(userdata)->on_expose(widget, event);
  }

private:
  VirtualKeyboard(const VirtualKeyboard&);
  VirtualKeyboard& operator=(const VirtualKeyboard&);
};

#endif

/* EOF */
