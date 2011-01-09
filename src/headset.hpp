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

#ifndef HEADER_XBOXDRV_HEADSET_HPP
#define HEADER_XBOXDRV_HEADSET_HPP

#include <memory>
#include <usb.h>
#include <boost/thread.hpp>

class Headset
{
private:
  struct usb_dev_handle* m_handle;
  std::auto_ptr<boost::thread> m_read_thread;
  std::auto_ptr<boost::thread> m_write_thread;

  bool m_quit_read_thread;
  bool m_quit_write_thread;

public:
  Headset(struct usb_dev_handle* handle, 
          bool debug,
          const std::string& dump_filename,
          const std::string& play_filename);
  ~Headset();

private:
  void write_thread(const std::string& filename);
  void read_thread(const std::string& filename, bool debug);

private:
  Headset(const Headset&);
  Headset& operator=(const Headset&);
};

#endif

/* EOF */
