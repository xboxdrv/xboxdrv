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

#ifndef HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_SEQUENCE_HPP
#define HEADER_XBOXDRV_BUTTONEVENT_CYCLE_KEY_SEQUENCE_HPP

#include <boost/shared_ptr.hpp>
#include <vector>
#include <assert.h>

#include "uinput/ui_event_sequence.hpp"

class CycleKeySequence;

typedef boost::shared_ptr<CycleKeySequence> CycleKeySequencePtr;

class CycleKeySequence
{
public:
  static CycleKeySequencePtr from_range(UInput& uinput, int slot, bool extra_devices,
                                        std::vector<std::string>::const_iterator beg,
                                        std::vector<std::string>::const_iterator end,
                                        bool wrap_around);

private:
  typedef std::vector<UIEventSequence> Keys;
  Keys m_keys;

  /** If set the sequence wraps around when at the begin/end */
  bool m_wrap_around;

  /** the position of the cursor in the sequence, if -1, it is unset */
  int m_current_key;

  /** the last key that was send out */
  int m_last_key;

public:
  CycleKeySequence(UInput& uinput, int slot, bool extra_devices,
                   const Keys& keys, bool wrap_around);

  bool has_current_key() const { return m_current_key != -1; }

  void next_key();
  void prev_key();

  void send(bool value);

private:
  CycleKeySequence(const CycleKeySequence&);
  CycleKeySequence& operator=(const CycleKeySequence&);
};

#endif

/* EOF */
