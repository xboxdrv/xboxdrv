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

#ifndef HEADER_XBOXDRV_UI_EVENT_EMITTER_HPP
#define HEADER_XBOXDRV_UI_EVENT_EMITTER_HPP

#include <boost/shared_ptr.hpp>
#include <stdint.h>

class UInput;
class UIEventEmitter;
class UIEventCollector;

typedef boost::shared_ptr<UIEventEmitter> UIEventEmitterPtr;

class UIEventEmitter
{
public:
  UIEventEmitter() {}
  virtual ~UIEventEmitter() {}

  virtual void send(int value) = 0;

private:
  UIEventEmitter(const UIEventEmitter&);
  UIEventEmitter& operator=(const UIEventEmitter&);
};

class UIRelEmitter
{
private:

public:
  UIRelEmitter();

};

#endif

/* EOF */
