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

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <memory>
#include <stdexcept>

#include "xpad_device.hpp"
#include "xboxdrv.hpp"
#include "linux_uinput.hpp"
#include "evdev_helper.hpp"

class Xbox360Msg;
class Xbox360GuitarMsg;
class XboxMsg;

struct ButtonEvent
{
  static ButtonEvent invalid() { return create(-1, -1); }
  static ButtonEvent create(int type, int code);
  static ButtonEvent from_string(const std::string& str);

  /** EV_KEY, EV_ABS, EV_REL */
  int type;

  /** BTN_A, REL_X, ABS_X, ... */
  int code;

  union {
    struct {
      int  repeat;
      int  value;
    } rel;

    struct {
      int value;
    } abs;

    struct {
    } key;
  };
};

struct AxisEvent
{
  static AxisEvent invalid() { return create(-1, -1); }
  static AxisEvent create(int type, int code);
  static AxisEvent from_string(const std::string& str);

  /** EV_KEY, EV_ABS, EV_REL */
  int type;

  /** BTN_A, REL_X, ABS_X, ... */
  int code;

  union {
    struct {
      int   repeat;
      float value;
    } rel;

    struct {
    } abs;

    struct {
      int secondary_code;
      int threshold;
    } key;
  };
};
  
class uInputCfg
{
public:
  std::string device_name;
  bool trigger_as_button;
  bool dpad_as_button;
  bool trigger_as_zaxis;
  bool dpad_only;
  bool force_feedback;
  bool extra_devices;

  ButtonEvent btn_map[XBOX_BTN_MAX];
  AxisEvent   axis_map[XBOX_AXIS_MAX];

  uInputCfg();
};
  
class uInput
{
private:
  std::auto_ptr<LinuxUinput> joystick_uinput_dev;
  std::auto_ptr<LinuxUinput> keyboard_uinput_dev;
  std::auto_ptr<LinuxUinput> mouse_uinput_dev;
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

  void setup_xbox360_gamepad(GamepadType type);
  void setup_xbox360_guitar();

  void send(XboxGenericMsg& msg); 
  void send(Xbox360Msg& msg);
  void send(Xbox360GuitarMsg& msg);
  void send(XboxMsg& msg);

  void add_axis(int code, int min, int max);
  void add_button(int code);

  void add_key(int ev_code);
  void send_key(int ev_code, bool value);

  void send_button(int code, bool value);
  void send_axis(int code, int32_t value);

  void update(int msec_delta);

  void set_ff_callback(const boost::function<void (uint8_t, uint8_t)>& callback);

  LinuxUinput* get_mouse_uinput() const;
  LinuxUinput* get_keyboard_uinput() const;
  LinuxUinput* get_joystick_uinput() const;

  bool need_mouse_device();
  bool need_keyboard_device();
  bool need_joystick_device();

  bool is_mouse_button(int ev_code);
  bool is_keyboard_button(int ev_code);
};

#endif

/* EOF */
