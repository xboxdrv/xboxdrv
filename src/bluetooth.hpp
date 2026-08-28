/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_XBOXDRV_BLUETOOTH_HPP
#define HEADER_XBOXDRV_BLUETOOTH_HPP

// Only used by the Wiimote backend (cwiid). Without HAVE_CWIID this
// header is a no-op so the tree builds without bluez headers.
#ifdef HAVE_CWIID

#include <bluetooth/bluetooth.h>

namespace xboxdrv {

class Bluetooth
{
private:
public:
  Bluetooth();

  static const bdaddr_t addr_any;
  static const bdaddr_t addr_all;
  static const bdaddr_t addr_local;

private:
  Bluetooth(const Bluetooth&);
  Bluetooth& operator=(const Bluetooth&);
};

} // namespace xboxdrv

#endif /* HAVE_CWIID */

#endif

/* EOF */
