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

#ifndef HEADER_XBOXDRV_WAKEUP_PIPE_HPP
#define HEADER_XBOXDRV_WAKEUP_PIPE_HPP

class WakeupPipe
{
private:
  int m_pipefd[2];

public:
  WakeupPipe();

  int get_read_fd() const;
  int get_write_fd() const;

  void send_wakeup();

private:
  WakeupPipe(const WakeupPipe&);
  WakeupPipe& operator=(const WakeupPipe&);
};

#endif

/* EOF */
