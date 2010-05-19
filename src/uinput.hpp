/* 
**  Xbox360 USB Gamepad Userspace Driver
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

#ifndef HEADER_UINPUT_HPP
#define HEADER_UINPUT_HPP

#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

#include "axis_event.hpp"
#include "button_event.hpp"
#include "evdev_helper.hpp"
#include "linux_uinput.hpp"
#include "uinput_cfg.hpp"
#include "xboxdrv.hpp"
#include "xpad_device.hpp"

class Xbox360Msg;
class Xbox360GuitarMsg;
class XboxMsg;
  
class uInput
{
private:
  XPadDevice m_dev;

  typedef std::map<int, boost::shared_ptr<LinuxUinput> > uInputDevs;
  uInputDevs uinput_devs;
  uInputCfg cfg;

  int  axis_state[XBOX_AXIS_MAX];
  bool button_state[XBOX_BTN_MAX];

  struct RelAxisState {
    int axis;
    int time;
    int next_time;
  };

  struct RelButtonState {
    int button;
    int time;
    int next_time;
  };

  // rel_axis[XBOx_AXIS_??] = ...
  std::vector<RelAxisState>   rel_axis;
  std::vector<RelButtonState> rel_button;

public:
  uInput(const XPadDevice& dev, uInputCfg cfg = uInputCfg());
  ~uInput();

  void send(XboxGenericMsg& msg); 
  void update(int msec_delta);
  void set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback);

private:
  void setup_xbox360_gamepad(GamepadType type);
  void setup_xbox360_guitar();

  void send(Xbox360Msg& msg);
  void send(Xbox360GuitarMsg& msg);
  void send(XboxMsg& msg);

  void add_axis(int code, int min, int max);
  void add_button(int code);

  void send_button(int code, bool value);
  void send_axis(int code, int32_t value);

  void add_key(int device_id, int ev_code);
  void send_key(int device_id, int ev_code, bool value);

  LinuxUinput* get_uinput(int device_id) const;
  LinuxUinput* get_mouse_uinput() const;
  LinuxUinput* get_force_feedback_uinput() const;

  int create_uinput_device(int device_id);
  int create_uinput_device(const AxisEvent& event);
  int create_uinput_device(const ButtonEvent& event);

  bool need_mouse_device();
  bool need_keyboard_device();
  bool need_joystick_device();

  bool is_mouse_button(int ev_code);
  bool is_keyboard_button(int ev_code);
};

#endif

/* EOF */
