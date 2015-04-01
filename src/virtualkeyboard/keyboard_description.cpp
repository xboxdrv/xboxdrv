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

#include "keyboard_description.hpp"

#include <assert.h>
#include <linux/input.h>

KeyboardDescriptionPtr
KeyboardDescription::create_us_layout()
{
  KeyboardDescriptionPtr desc(new KeyboardDescription(21, 6));

  // row 0
  desc->make_key( 0, 0, KEY_ESC, Key::kModifier, "Esc");
  desc->make_key( 1, 0, KEY_F1, Key::kFunction, "F1");
  desc->make_key( 2, 0, KEY_F2, Key::kFunction, "F2");
  desc->make_key( 3, 0, KEY_F3, Key::kFunction, "F3");
  desc->make_key( 4, 0, KEY_F4, Key::kFunction, "F4");
  desc->make_key( 5, 0, KEY_F5, Key::kFunction, "F5");
  desc->make_key( 6, 0, KEY_F6, Key::kFunction, "F6");
  desc->make_key( 7, 0, KEY_F7, Key::kFunction, "F7");
  desc->make_key( 8, 0, KEY_F8, Key::kFunction, "F8");
  desc->make_key( 9, 0, KEY_F9, Key::kFunction, "F9");
  desc->make_key(10, 0, KEY_F10, Key::kFunction, "F10");
  desc->make_key(11, 0, KEY_F11, Key::kFunction, "F11");
  desc->make_key(12, 0, KEY_F12, Key::kFunction, "F12");
  //desc->make_key(13, 0, 0);
  desc->make_key(14, 0, KEY_PRINT, Key::kModifier, "Print");
  desc->make_key(15, 0, KEY_SCROLLLOCK, Key::kModifier, "Scroll");
  desc->make_key(16, 0, KEY_PAUSE, Key::kModifier, "Pause");
  desc->make_key(17, 0, KEY_MUTE, Key::kModifier, "Mute");
  desc->make_key(18, 0, KEY_VOLUMEDOWN, Key::kModifier, "Vol-");
  desc->make_key(19, 0, KEY_VOLUMEUP, Key::kModifier, "Vol+");
  desc->make_key(20, 0, KEY_PLAYPAUSE, Key::kModifier, "Play");

  // row 1
  desc->make_key( 0, 1, KEY_APOSTROPHE, Key::kLetter, "`", "~");
  desc->make_key( 1, 1, KEY_1, Key::kLetter, "1", "!");
  desc->make_key( 2, 1, KEY_2, Key::kLetter, "2", "@");
  desc->make_key( 3, 1, KEY_3, Key::kLetter, "3", "#");
  desc->make_key( 4, 1, KEY_4, Key::kLetter, "4", "$");
  desc->make_key( 5, 1, KEY_5, Key::kLetter, "5", "%");
  desc->make_key( 6, 1, KEY_6, Key::kLetter, "6", "^");
  desc->make_key( 7, 1, KEY_7, Key::kLetter, "7", "&");
  desc->make_key( 8, 1, KEY_8, Key::kLetter, "8", "*");
  desc->make_key( 9, 1, KEY_9, Key::kLetter, "9", "(");
  desc->make_key(10, 1, KEY_0, Key::kLetter, "0", ")");
  desc->make_key(11, 1, KEY_MINUS, Key::kLetter, "-", "_");
  desc->make_key(12, 1, KEY_EQUAL, Key::kLetter, "=", "+");
  desc->make_key(13, 1, KEY_BACKSPACE, Key::kModifier, "BkSp"); //"\u232B");
  desc->make_key(14, 1, KEY_INSERT, Key::kModifier, "Ins");
  desc->make_key(15, 1, KEY_HOME, Key::kModifier, "Home");
  desc->make_key(16, 1, KEY_PAGEUP, Key::kModifier, "PgUp");
  desc->make_key(17, 1, KEY_NUMLOCK, Key::kModifier, "Num");
  desc->make_key(18, 1, KEY_KPSLASH, Key::kLetter, "/");
  desc->make_key(19, 1, KEY_KPASTERISK, Key::kLetter, "*");
  desc->make_key(20, 1, KEY_KPMINUS, Key::kLetter, "-");

  // row 2
  desc->make_key( 0, 2, KEY_TAB, Key::kModifier, "Tab"); ///"\u21E5\u21E4");
  desc->make_key( 1, 2, KEY_Q, Key::kLetter, "q", "Q");
  desc->make_key( 2, 2, KEY_W, Key::kLetter, "w", "W");
  desc->make_key( 3, 2, KEY_E, Key::kLetter, "e", "E");
  desc->make_key( 4, 2, KEY_R, Key::kLetter, "r", "R");
  desc->make_key( 5, 2, KEY_T, Key::kLetter, "t", "T");
  desc->make_key( 6, 2, KEY_Y, Key::kLetter, "y", "Y");
  desc->make_key( 7, 2, KEY_U, Key::kLetter, "u", "U");
  desc->make_key( 8, 2, KEY_I, Key::kLetter, "i", "I");
  desc->make_key( 9, 2, KEY_O, Key::kLetter, "o", "O");
  desc->make_key(10, 2, KEY_P, Key::kLetter, "p", "P");
  desc->make_key(11, 2, KEY_LEFTBRACE, Key::kLetter, "[", "{");
  desc->make_key(12, 2, KEY_RIGHTBRACE, Key::kLetter, "]", "}");
  desc->make_key(13, 2, KEY_BACKSLASH, Key::kLetter, "\\", "|");
  desc->make_key(14, 2, KEY_DELETE, Key::kModifier, "Del");
  desc->make_key(15, 2, KEY_END, Key::kModifier, "End");
  desc->make_key(16, 2, KEY_PAGEDOWN, Key::kModifier, "PgDn");
  desc->make_key(17, 2, KEY_KP7, Key::kLetter, "7");
  desc->make_key(18, 2, KEY_KP8, Key::kLetter, "8");
  desc->make_key(19, 2, KEY_KP9, Key::kLetter, "9");
  desc->make_key(20, 2, KEY_KPPLUS, Key::kLetter, "+", "", "", 1, 2);

  // row 3
  desc->make_key( 0, 3, KEY_CAPSLOCK, Key::kModifier, "Caps");
  desc->make_key( 1, 3, KEY_A, Key::kLetter, "a", "A");
  desc->make_key( 2, 3, KEY_S, Key::kLetter, "s", "S");
  desc->make_key( 3, 3, KEY_D, Key::kLetter, "d", "D");
  desc->make_key( 4, 3, KEY_F, Key::kLetter, "f", "F");
  desc->make_key( 5, 3, KEY_G, Key::kLetter, "g", "G");
  desc->make_key( 6, 3, KEY_H, Key::kLetter, "h", "H");
  desc->make_key( 7, 3, KEY_J, Key::kLetter, "j", "J");
  desc->make_key( 8, 3, KEY_K, Key::kLetter, "k", "K");
  desc->make_key( 9, 3, KEY_L, Key::kLetter, "l", "L");
  desc->make_key(10, 3, KEY_SEMICOLON, Key::kLetter, ";", ":");
  desc->make_key(11, 3, KEY_APOSTROPHE, Key::kLetter, "'", "\"");
  desc->make_key(12, 3, KEY_ENTER, Key::kModifier, "Enter", "", "", 2, 1);
  //desc->make_key(13, 3, 0);
  //desc->make_key(14, 3, 0);
  //desc->make_key(15, 3, 0);
  //desc->make_key(16, 3, 0);
  desc->make_key(17, 3, KEY_KP4, Key::kLetter, "4");
  desc->make_key(18, 3, KEY_KP5, Key::kLetter, "5");
  desc->make_key(19, 3, KEY_KP6, Key::kLetter, "6");
  //desc->make_key(20, 3, 0);

  // row 4
  desc->make_key( 0, 4, KEY_LEFTSHIFT, Key::kModifier, "Shift", "", "", 2, 1);
  //desc->make_key( 1, 4, 0);
  desc->make_key( 2, 4, KEY_Y, Key::kLetter, "y", "Y");
  desc->make_key( 3, 4, KEY_X, Key::kLetter, "x", "X");
  desc->make_key( 4, 4, KEY_C, Key::kLetter, "c", "C");
  desc->make_key( 5, 4, KEY_V, Key::kLetter, "v", "V");
  desc->make_key( 6, 4, KEY_B, Key::kLetter, "b", "B");
  desc->make_key( 7, 4, KEY_N, Key::kLetter, "n", "N");
  desc->make_key( 8, 4, KEY_M, Key::kLetter, "m", "M");
  desc->make_key( 9, 4, KEY_COMMA, Key::kLetter, ",", "<");
  desc->make_key(10, 4, KEY_DOT, Key::kLetter, ".", ">");
  desc->make_key(11, 4, KEY_SLASH, Key::kLetter, "/", "?");
  desc->make_key(12, 4, KEY_RIGHTSHIFT, Key::kModifier, "Shift", "", "", 2, 1);
  //desc->make_key(13, 4, 0);
  //desc->make_key(14, 4, 0);
  desc->make_key(15, 4, KEY_UP, Key::kLetter, "\u2191");
  //desc->make_key(16, 4, 0);
  desc->make_key(17, 4, KEY_KP1, Key::kLetter, "1");
  desc->make_key(18, 4, KEY_KP2, Key::kLetter, "2");
  desc->make_key(19, 4, KEY_KP3, Key::kLetter, "3");
  desc->make_key(20, 4, KEY_KPENTER, Key::kModifier, "Enter", "", "", 1, 2);

  // row 5
  desc->make_key( 0, 5, KEY_LEFTCTRL, Key::kModifier, "Ctrl", "", "", 2, 1);
  desc->make_key( 2, 5, KEY_LEFTMETA, Key::kModifier, "Win");
  desc->make_key( 3, 5, KEY_LEFTALT, Key::kModifier, "Alt", "", "", 2, 1);
  desc->make_key( 5, 5, KEY_SPACE, Key::kModifier, "Space", "", "", 3, 1);
  desc->make_key( 8, 5, KEY_RIGHTALT, Key::kModifier, "Alt", "", "", 2, 1);
  desc->make_key(10, 5, KEY_RIGHTMETA, Key::kModifier, "Win");
  desc->make_key(11, 5, KEY_COMPOSE, Key::kModifier, "Menu");
  desc->make_key(12, 5, KEY_RIGHTCTRL, Key::kModifier, "Ctrl", "", "", 2, 1);
  //desc->make_key(13, 5, 0);
  desc->make_key(14, 5, KEY_LEFT, Key::kLetter, "\u2190");
  desc->make_key(15, 5, KEY_DOWN, Key::kLetter, "\u2193");
  desc->make_key(16, 5, KEY_RIGHT, Key::kLetter, "\u2192");
  desc->make_key(17, 5, KEY_KP0, Key::kLetter, "0", "", "", 2, 1);
  //desc->make_key(18, 5, 0);
  desc->make_key(19, 5, KEY_KPDOT, Key::kLetter, ".");
  //desc->make_key(20, 5, 0);

  return desc;
}

KeyboardDescription::KeyboardDescription(int width, int height) :
  m_width(width),
  m_height(height),
  m_keys(width * height)
{
}

KeyboardDescription::~KeyboardDescription()
{
  for(std::vector<Key*>::const_iterator i = m_keys.begin(); i != m_keys.end(); ++i)
  {
    delete *i;
  }
  m_keys.clear();
}

void
KeyboardDescription::make_key(int x, int y,
                              int code,
                              Key::Style style,
                              const std::string& label,
                              const std::string& shift_label,
                              const std::string& alt_label,
                              int xspan,
                              int yspan)
{
  set_key( x, y, new RealKey(x, y, code, style, label, shift_label, alt_label, xspan, yspan));
}

Key*
KeyboardDescription::get_key(int x, int y) const
{
  return m_keys[m_width * y + x];
}

void
KeyboardDescription::set_key(int x, int y, Key* key)
{
  assert(m_keys[m_width * y + x] == 0);
  //m_keys[m_width * y + x] = key;

  // mark the spots that a large key covers with ReferenceKey
  if (key && !key->is_ref_key())
  {
    for(int iy = 0; iy < key->get_yspan(); ++iy)
      for(int ix = 0; ix < key->get_xspan(); ++ix)
      {
        if (ix == 0 && iy == 0)
        {
          m_keys[m_width * (y+iy) + (x+ix)] = key;
        }
        else
        {
          m_keys[m_width * (y+iy) + (x+ix)] = new ReferenceKey(key);
        }
      }
  }
}

/* EOF */
