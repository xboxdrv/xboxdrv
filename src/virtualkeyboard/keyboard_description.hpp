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

#ifndef HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DESCRIPTION_HPP
#define HEADER_XBOXDRV_VIRTUALKEYBOARD_KEYBOARD_DESCRIPTION_HPP

#include <vector>
#include <string>
#include <assert.h>
#include <boost/shared_ptr.hpp>

class Key
{
public:
  enum Style { kLetter, kFunction, kModifier };

public:
  virtual ~Key() {}

  virtual int get_code() const =0;

  virtual int get_x() const =0;
  virtual int get_y() const =0;

  virtual int get_xspan() const =0;
  virtual int get_yspan() const =0;

  virtual Style get_style() const =0;

  virtual std::string get_label() const =0;
  virtual std::string get_shift_label() const =0;
  virtual std::string get_alt_label() const =0;

  virtual bool is_ref_key() const =0;
  virtual Key* get_parent() =0;
};

class RealKey : public Key
{
public:
  //UIEventSequence m_sequence;
  int m_x;
  int m_y;
  int m_code;
  Style m_style;
  int m_xspan;
  int m_yspan;

  std::string m_label;
  std::string m_shift_label;
  std::string m_alt_label;

  RealKey(int x, int y,
          int code,
          Style style,
          const std::string& label,
          const std::string& shift_label = std::string(),
          const std::string& alt_label = std::string(),
          int xspan = 1,
          int yspan = 1) :
    m_x(x),
    m_y(y),
    m_code(code),
    m_style(style),
    m_xspan(xspan),
    m_yspan(yspan),
    m_label(label),
    m_shift_label(shift_label),
    m_alt_label(alt_label)
  {}

  int get_code() const { return m_code; }

  int get_x() const { return m_x; }
  int get_y() const { return m_y; }

  int get_xspan() const { return m_xspan; }
  int get_yspan() const { return m_yspan; }

  Style get_style() const { return m_style; }

  std::string get_label() const { return m_label; }
  std::string get_shift_label() const { return m_shift_label; }
  std::string get_alt_label() const { return m_alt_label; }

  bool is_ref_key() const { return false; }
  Key* get_parent() { return this; }
};

class ReferenceKey : public Key
{
private:
  Key* m_key;

public:
  ReferenceKey(Key* key) :
    m_key(key)
  {
  }

  int get_code() const { return m_key->get_code(); }

  int get_x() const { return m_key->get_x(); }
  int get_y() const { return m_key->get_y(); }

  int get_xspan() const { return m_key->get_xspan(); }
  int get_yspan() const { return m_key->get_yspan(); }

  Style get_style() const { return m_key->get_style(); }

  std::string get_label() const { return m_key->get_label(); }
  std::string get_shift_label() const { return m_key->get_shift_label(); }
  std::string get_alt_label() const { return m_key->get_alt_label(); }

  bool is_ref_key() const { return true; }
  Key* get_parent() { return m_key; }

private:
  ReferenceKey(const ReferenceKey&);
  ReferenceKey& operator=(const ReferenceKey&);
};

class KeyboardDescription;

typedef boost::shared_ptr<KeyboardDescription> KeyboardDescriptionPtr;

class KeyboardDescription
{
public:
  static KeyboardDescriptionPtr create_us_layout();

private:
  int m_width;
  int m_height;

  std::vector<Key*> m_keys;

private:
  KeyboardDescription(int width, int height);

public:
  ~KeyboardDescription();

  int get_width() const  { return m_width; }
  int get_height() const { return m_height; }

  Key* get_key(int x, int y) const;

private:
  void make_key(int x, int y,
                int code,
                Key::Style style,
                const std::string& label,
                const std::string& shift_label = std::string(),
                const std::string& alt_label = std::string(),
                int xspan = 1,
                int yspan = 1);
  void set_key(int x, int y, Key* key);

private:
  KeyboardDescription(const KeyboardDescription&);
  KeyboardDescription& operator=(const KeyboardDescription&);
};

#endif

/* EOF */
