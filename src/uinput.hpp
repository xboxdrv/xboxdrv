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

#include <boost/thread/mutex.hpp>

#include "axis_event.hpp"
#include "linux_uinput.hpp"

struct Xbox360Msg;
struct XboxMsg;
struct Xbox360GuitarMsg;
  
class UInput
{
public:
  static struct input_id parse_input_id(const std::string& str);
  static uint32_t parse_device_id(const std::string& str);

  static inline uint32_t create_device_id(uint16_t slot_id, uint16_t type_id)
  {
    return (slot_id << 16) | type_id;
  }

  static inline uint16_t get_type_id(uint32_t device_id)
  {
    return device_id & 0xffff;
  }

  static inline uint16_t get_slot_id(uint32_t device_id)
  {
    return ((device_id) >> 16) & 0xffff;
  }

private:
  typedef std::map<uint32_t, boost::shared_ptr<LinuxUinput> > UInputDevs;
  UInputDevs m_uinput_devs;

  typedef std::map<uint32_t, std::string> DeviceNames;
  DeviceNames m_device_names;

  typedef std::map<uint32_t, struct input_id> DeviceUSBId;
  DeviceUSBId m_device_usbids;

  struct RelRepeat 
  {
    UIEvent code;
    float value;
    float rest;
    int time_count;
    int repeat_interval;
  };

  std::map<UIEvent, RelRepeat> m_rel_repeat_lst;

  boost::mutex m_mutex;
  bool m_extra_events;

public:
  UInput(bool extra_events);
  ~UInput();

  void update(int msec_delta);

  void set_device_names(const std::map<uint32_t, std::string>& device_names);
  void set_device_usbids(const std::map<uint32_t, struct input_id>& device_usbids);
  void set_ff_callback(int device_id, const boost::function<void (uint8_t, uint8_t)>& callback);

  /** Device construction functions
      @{*/
  void add_rel(uint32_t device_id, int ev_code);
  void add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat);
  void add_key(uint32_t device_id, int ev_code);
  void add_ff(uint32_t device_id, uint16_t code);

  /** needs to be called to finish device creation and create the
      device in the kernel */
  void finish();
  /** @} */

  /** Send events to the kernel
      @{*/
  void send(uint32_t device_id, int ev_type, int ev_code, int value);
  void send_abs(uint32_t device_id, int ev_code, int value);
  void send_key(uint32_t device_id, int ev_code, bool value);
  void send_rel(uint32_t device_id, int ev_code, bool value);
  void send_rel_repetitive(const UIEvent& code, float value, int repeat_interval);

  /** should be called to single that all events of the current frame
      have been send */
  void sync();
  /** @} */

  boost::mutex& get_mutex() { return m_mutex; }

private:
  /** create a LinuxUinput with the given device_id, if some already
      exist return a pointer to it */
  LinuxUinput* create_uinput_device(uint32_t device_id);

  /** must only be called with a valid device_id */
  LinuxUinput* get_uinput(uint32_t device_id) const;

  std::string get_device_name(uint32_t device_id) const;
  struct input_id get_device_usbid(uint32_t device_id) const;
};

#endif

/* EOF */
