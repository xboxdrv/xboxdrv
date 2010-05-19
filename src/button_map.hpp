/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOXDRV_BUTTON_MAP_HPP
#define HEADER_XBOXDRV_BUTTON_MAP_HPP

#include <assert.h>

#include "button_event.hpp"
#include "xboxmsg.hpp"

class ButtonMap
{
private:
  ButtonEvent btn_map[XBOX_BTN_MAX][XBOX_BTN_MAX];
  
public:
  ButtonMap();

  void bind(int code, const ButtonEvent& event);
  void bind(int shift_code, int code, const ButtonEvent& event);

  ButtonEvent lookup(int code) const;
  ButtonEvent lookup(int shift_code, int code) const;

  void clear();

  template<class Pred>
  bool contains(const Pred& pred) 
  {
    for(int shift_code = 0; shift_code < XBOX_BTN_MAX; ++shift_code)
    {
      for(int code = 0; code < XBOX_BTN_MAX; ++code)
      {
        if (pred(btn_map[shift_code][code]))
        {
          return true;
        }
      }
    }
    return false;
  }

  // FIXME: use boost::iterator_apdator or a template to make this
  // prettier and allow proper const, non-const versions
  struct iterator 
  {
    friend class ButtonMap;
  private:
    ButtonMap& btn_map;
    int j;
    int i;

    iterator(ButtonMap& btn_map_) :
      btn_map(btn_map_), 
      j(0), 
      i(0)
    {}

    iterator(ButtonMap& btn_map_, int j_, int i_) :
      btn_map(btn_map_), 
      j(j_), 
      i(i_)
    {}

  public:
    void operator++()
    {
      assert(j < XBOX_BTN_MAX);

      i += 1;
      if (i == XBOX_BTN_MAX)
      {
        i  = 0;
        j += 1;
      }
    }

    const ButtonEvent& operator*() const
    {
      return btn_map.btn_map[j][i];
    }

    const ButtonEvent& operator->() const
    {
      return btn_map.btn_map[j][i];
    }

    ButtonEvent& operator*()
    {
      return btn_map.btn_map[j][i];
    }

    ButtonEvent& operator->()
    {
      return btn_map.btn_map[j][i];
    }

    bool operator!=(const iterator& rhs) const
    {
      assert(&btn_map == &rhs.btn_map);
      return (j != rhs.j || i != rhs.i);
    }
  };

  iterator begin() { return iterator(*this); }
  iterator end()   { return iterator(*this, XBOX_BTN_MAX, 0); }
};

#endif

/* EOF */
