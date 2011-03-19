/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_XBOX_GENERIC_CONTROLLER_HPP
#define HEADER_XBOX_GENERIC_CONTROLLER_HPP

#include <stdint.h>

#include <boost/function.hpp>
#include <memory>
#include <libudev.h>

class MessageProcessor;
struct XboxGenericMsg;

class Controller
{
private:
  std::auto_ptr<MessageProcessor> m_processor;
  udev_device* m_udev_device;

public:
  Controller();
  virtual ~Controller();

  virtual void start() {}
  virtual void stop()  {}

  virtual void set_rumble(uint8_t left, uint8_t right) =0;
  virtual void set_led(uint8_t status)   =0;

  /**
     @param timeout   timeout in msec, 0 means forever 
     @return true if something was read, false otherwise
   */  
  virtual bool read(XboxGenericMsg& msg, int timeout) =0;

  virtual bool is_active() const { return true; }
  virtual void set_activation_cb(const boost::function<void ()> callback) {}

  virtual std::string get_usbpath() const { return "-1:-1"; }
  virtual std::string get_usbid() const   { return "-1:-1"; }
  virtual std::string get_name() const    { return "<not implemented>"; }

  void set_message_proc(std::auto_ptr<MessageProcessor> processor);
  MessageProcessor* get_message_proc() const { return m_processor.get(); }

  void set_udev_device(udev_device* udev_dev);
  udev_device* get_udev_device() const;

  void submit_msg(const XboxGenericMsg& msg);

private:
  Controller (const Controller&);
  Controller& operator= (const Controller&);
};

#endif

/* EOF */
