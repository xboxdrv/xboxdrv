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

#include <linux/input.h>

KeyboardDescription
KeyboardDescription::create_us_layout()
{
  KeyboardDescription desc(21, 6);

  // row 0
  desc.set_key( 0, 0, Key(KEY_ESC, Key::kModifier, "Esc"));
  desc.set_key( 1, 0, Key(KEY_F1, Key::kFunction, "F1"));
  desc.set_key( 2, 0, Key(KEY_F2, Key::kFunction, "F2"));
  desc.set_key( 3, 0, Key(KEY_F3, Key::kFunction, "F3"));
  desc.set_key( 4, 0, Key(KEY_F4, Key::kFunction, "F4"));
  desc.set_key( 5, 0, Key(KEY_F5, Key::kFunction, "F5"));
  desc.set_key( 6, 0, Key(KEY_F6, Key::kFunction, "F6"));
  desc.set_key( 7, 0, Key(KEY_F7, Key::kFunction, "F7"));
  desc.set_key( 8, 0, Key(KEY_F8, Key::kFunction, "F8"));
  desc.set_key( 9, 0, Key(KEY_F9, Key::kFunction, "F9"));
  desc.set_key(10, 0, Key(KEY_F10, Key::kFunction, "F10"));
  desc.set_key(11, 0, Key(KEY_F11, Key::kFunction, "F11"));
  desc.set_key(12, 0, Key(KEY_F12, Key::kFunction, "F12"));
  desc.set_key(13, 0, Key());
  desc.set_key(14, 0, Key(KEY_PRINT, Key::kModifier, "Print"));
  desc.set_key(15, 0, Key(KEY_SCROLLLOCK, Key::kModifier, "Scroll"));
  desc.set_key(16, 0, Key(KEY_PAUSE, Key::kModifier, "Pause"));
  desc.set_key(17, 0, Key(KEY_MUTE, Key::kModifier, "Mute"));
  desc.set_key(18, 0, Key(KEY_VOLUMEDOWN, Key::kModifier, "Vol-"));
  desc.set_key(19, 0, Key(KEY_VOLUMEUP, Key::kModifier, "Vol+"));
  desc.set_key(20, 0, Key(KEY_PLAYPAUSE, Key::kModifier, "Play"));

  // row 1
  desc.set_key( 0, 1, Key(KEY_APOSTROPHE, Key::kLetter, "`", "~"));
  desc.set_key( 1, 1, Key(KEY_1, Key::kLetter, "1", "!"));
  desc.set_key( 2, 1, Key(KEY_2, Key::kLetter, "2", "@"));
  desc.set_key( 3, 1, Key(KEY_3, Key::kLetter, "3", "#"));
  desc.set_key( 4, 1, Key(KEY_4, Key::kLetter, "4", "$"));
  desc.set_key( 5, 1, Key(KEY_5, Key::kLetter, "5", "%"));
  desc.set_key( 6, 1, Key(KEY_6, Key::kLetter, "6", "^"));
  desc.set_key( 7, 1, Key(KEY_7, Key::kLetter, "7", "&"));
  desc.set_key( 8, 1, Key(KEY_8, Key::kLetter, "8", "*"));
  desc.set_key( 9, 1, Key(KEY_9, Key::kLetter, "9", "("));
  desc.set_key(10, 1, Key(KEY_0, Key::kLetter, "0", ")"));
  desc.set_key(11, 1, Key(KEY_MINUS, Key::kLetter, "-", "_"));
  desc.set_key(12, 1, Key(KEY_EQUAL, Key::kLetter, "=", "+"));
  desc.set_key(13, 1, Key(KEY_BACKSPACE, Key::kModifier, "BkSp")); //"\u232B"));
  desc.set_key(14, 1, Key(KEY_INSERT, Key::kModifier, "Ins"));
  desc.set_key(15, 1, Key(KEY_HOME, Key::kModifier, "Home"));
  desc.set_key(16, 1, Key(KEY_PAGEUP, Key::kModifier, "PgUp"));
  desc.set_key(17, 1, Key(KEY_NUMLOCK, Key::kModifier, "Num"));
  desc.set_key(18, 1, Key(KEY_KPSLASH, Key::kLetter, "/"));
  desc.set_key(19, 1, Key(KEY_KPASTERISK, Key::kLetter, "*"));
  desc.set_key(20, 1, Key(KEY_KPMINUS, Key::kLetter, "-"));
  
  // row 2
  desc.set_key( 0, 2, Key(KEY_TAB, Key::kModifier, "Tab")); ///"\u21E5\u21E4"));
  desc.set_key( 1, 2, Key(KEY_Q, Key::kLetter, "q", "Q"));
  desc.set_key( 2, 2, Key(KEY_W, Key::kLetter, "w", "W"));
  desc.set_key( 3, 2, Key(KEY_E, Key::kLetter, "e", "E"));
  desc.set_key( 4, 2, Key(KEY_R, Key::kLetter, "r", "R"));
  desc.set_key( 5, 2, Key(KEY_T, Key::kLetter, "t", "T"));
  desc.set_key( 6, 2, Key(KEY_Y, Key::kLetter, "y", "Y"));
  desc.set_key( 7, 2, Key(KEY_U, Key::kLetter, "u", "U"));
  desc.set_key( 8, 2, Key(KEY_I, Key::kLetter, "i", "I"));
  desc.set_key( 9, 2, Key(KEY_O, Key::kLetter, "o", "O"));
  desc.set_key(10, 2, Key(KEY_P, Key::kLetter, "p", "P"));
  desc.set_key(11, 2, Key(KEY_LEFTBRACE, Key::kLetter, "[", "{"));
  desc.set_key(12, 2, Key(KEY_RIGHTBRACE, Key::kLetter, "]", "}"));
  desc.set_key(13, 2, Key(KEY_BACKSLASH, Key::kLetter, "\\", "|"));
  desc.set_key(14, 2, Key(KEY_DELETE, Key::kModifier, "Del"));
  desc.set_key(15, 2, Key(KEY_END, Key::kModifier, "End"));
  desc.set_key(16, 2, Key(KEY_PAGEDOWN, Key::kModifier, "PgDn"));
  desc.set_key(17, 2, Key(KEY_KP7, Key::kLetter, "7"));
  desc.set_key(18, 2, Key(KEY_KP8, Key::kLetter, "8"));
  desc.set_key(19, 2, Key(KEY_KP9, Key::kLetter, "9"));
  desc.set_key(20, 2, Key(KEY_KPPLUS, Key::kLetter, "+", "", "", 1, 2));
  
  // row 3
  desc.set_key( 0, 3, Key(KEY_CAPSLOCK, Key::kModifier, "Caps"));
  desc.set_key( 1, 3, Key(KEY_A, Key::kLetter, "a", "A"));
  desc.set_key( 2, 3, Key(KEY_S, Key::kLetter, "s", "S"));
  desc.set_key( 3, 3, Key(KEY_D, Key::kLetter, "d", "D"));
  desc.set_key( 4, 3, Key(KEY_F, Key::kLetter, "f", "F"));
  desc.set_key( 5, 3, Key(KEY_G, Key::kLetter, "g", "G"));
  desc.set_key( 6, 3, Key(KEY_H, Key::kLetter, "h", "H"));
  desc.set_key( 7, 3, Key(KEY_J, Key::kLetter, "j", "J"));
  desc.set_key( 8, 3, Key(KEY_K, Key::kLetter, "k", "K"));
  desc.set_key( 9, 3, Key(KEY_L, Key::kLetter, "l", "L"));
  desc.set_key(10, 3, Key(KEY_SEMICOLON, Key::kLetter, ";", ":"));
  desc.set_key(11, 3, Key(KEY_APOSTROPHE, Key::kLetter, "'", "\""));
  desc.set_key(12, 3, Key(KEY_ENTER, Key::kModifier, "Enter", "", "", 2, 1));
  desc.set_key(13, 3, Key());
  desc.set_key(14, 3, Key());
  desc.set_key(15, 3, Key());
  desc.set_key(16, 3, Key());
  desc.set_key(17, 3, Key(KEY_KP4, Key::kLetter, "4"));
  desc.set_key(18, 3, Key(KEY_KP5, Key::kLetter, "5"));
  desc.set_key(19, 3, Key(KEY_KP6, Key::kLetter, "6"));
  desc.set_key(20, 3, Key());

  // row 4
  desc.set_key( 0, 4, Key(KEY_LEFTSHIFT, Key::kModifier, "Shift", "", "", 2, 1));
  desc.set_key( 1, 4, Key());
  desc.set_key( 2, 4, Key(KEY_Y, Key::kLetter, "y", "Y"));
  desc.set_key( 3, 4, Key(KEY_X, Key::kLetter, "x", "X"));
  desc.set_key( 4, 4, Key(KEY_C, Key::kLetter, "c", "C"));
  desc.set_key( 5, 4, Key(KEY_V, Key::kLetter, "v", "V"));
  desc.set_key( 6, 4, Key(KEY_B, Key::kLetter, "b", "B"));
  desc.set_key( 7, 4, Key(KEY_N, Key::kLetter, "n", "N"));
  desc.set_key( 8, 4, Key(KEY_M, Key::kLetter, "m", "M"));
  desc.set_key( 9, 4, Key(KEY_COMMA, Key::kLetter, ",", "<"));
  desc.set_key(10, 4, Key(KEY_DOT, Key::kLetter, ".", ">"));
  desc.set_key(11, 4, Key(KEY_SLASH, Key::kLetter, "/", "?"));
  desc.set_key(12, 4, Key(KEY_RIGHTSHIFT, Key::kModifier, "Shift", "", "", 2, 1));
  desc.set_key(13, 4, Key());
  desc.set_key(14, 4, Key());
  desc.set_key(15, 4, Key(KEY_UP, Key::kLetter, "\u2191"));
  desc.set_key(16, 4, Key());
  desc.set_key(17, 4, Key(KEY_KP1, Key::kLetter, "1"));
  desc.set_key(18, 4, Key(KEY_KP2, Key::kLetter, "2"));
  desc.set_key(19, 4, Key(KEY_KP3, Key::kLetter, "3"));
  desc.set_key(20, 4, Key(KEY_KPENTER, Key::kModifier, "Enter", "", "", 1, 2));

  // row 5
  desc.set_key( 0, 5, Key(KEY_LEFTCTRL, Key::kModifier, "Ctrl", "", "", 2, 1));
  desc.set_key( 2, 5, Key(KEY_LEFTMETA, Key::kModifier, "Win"));
  desc.set_key( 3, 5, Key(KEY_LEFTALT, Key::kModifier, "Alt", "", "", 2, 1));
  desc.set_key( 5, 5, Key(KEY_SPACE, Key::kModifier, "Space", "", "", 3, 1));
  desc.set_key( 8, 5, Key(KEY_RIGHTALT, Key::kModifier, "Alt", "", "", 2, 1));
  desc.set_key(10, 5, Key(KEY_RIGHTMETA, Key::kModifier, "Win"));
  desc.set_key(11, 5, Key(KEY_COMPOSE, Key::kModifier, "Menu"));
  desc.set_key(12, 5, Key(KEY_RIGHTCTRL, Key::kModifier, "Ctrl", "", "", 2, 1));
  desc.set_key(13, 5, Key());
  desc.set_key(14, 5, Key(KEY_LEFT, Key::kLetter, "\u2190"));
  desc.set_key(15, 5, Key(KEY_DOWN, Key::kLetter, "\u2193"));
  desc.set_key(16, 5, Key(KEY_RIGHT, Key::kLetter, "\u2192"));
  desc.set_key(17, 5, Key(KEY_KP0, Key::kLetter, "0", "", "", 2, 1));
  desc.set_key(18, 5, Key());
  desc.set_key(19, 5, Key(KEY_KPDOT, Key::kLetter, "."));
  desc.set_key(20, 5, Key());

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
}

/* EOF */
