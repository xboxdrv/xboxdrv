// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_UINPP_DEVICE_HPP
#define HEADER_UINPP_DEVICE_HPP

#include <cstdint>
#include <functional>
#include <linux/uinput.h>
#include <string>

#include "fwd.hpp"

namespace uinpp {

enum class DeviceType
{
  GENERIC,
  KEYBOARD,
  MOUSE,
  JOYSTICK
};

class Device
{
public:
  Device(DeviceType device_type, std::string const& name,
         const struct input_id& iid);
  ~Device();

  /*@{*/
  void set_phys(std::string_view phys);
  void set_prop(int value);

  /** Create an absolute axis */
  void add_abs(uint16_t code, int min, int max, int fuzz = 0, int flat = 0, int resolution = 0);

  /** Create an button */
  void add_key(uint16_t code);

  /** Create a relative axis (mice) */
  void add_rel(uint16_t code);

  void add_ff(uint16_t code);

  void set_ff_callback(const std::function<void (uint8_t, uint8_t)>& callback);

  /** Finalized the device creation */
  void finish();
  /*@}*/

  /** Send an input event */
  void send(uint16_t type, uint16_t code, int32_t value);

  /** Sends out a sync event if there is a need for it. */
  void sync();

  /** Update force feedback */
  void update(int msec_delta);

  /** Read incoming data (force feedback, led, etc.), non-blocking */
  void read();

  /** file handle to the underlying device */
  int get_fd() const { return m_fd; }

private:
  DeviceType  m_device_type;
  input_id m_iid;
  std::string m_name;

  bool m_finished;

  int m_fd;

  bool m_key_bit;
  bool m_rel_bit;
  bool m_abs_bit;
  bool m_led_bit;
  bool m_ff_bit;

  bool m_abs_lst[ABS_CNT];
  bool m_rel_lst[REL_CNT];
  bool m_key_lst[KEY_CNT];
  bool m_ff_lst[FF_CNT];

  ForceFeedbackHandler* m_ff_handler;
  std::function<void (uint8_t, uint8_t)> m_ff_callback;

  bool m_needs_sync;

private:
  Device (Device const&) = delete;
  Device& operator= (Device const&) = delete;
};

} // namespace uinpp

#endif

/* EOF */
