/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
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

#include <map>

#include "uinput/ui_event.hpp"
#include "uinput/linux_uinput.hpp"
#include "uinput/ui_event_emitter.hpp"
#include "uinput/ui_event_collector.hpp"

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
    return static_cast<uint16_t>(device_id & 0xffff);
  }

  static inline uint16_t get_slot_id(uint32_t device_id)
  {
    return static_cast<uint16_t>(((device_id) >> 16) & 0xffff);
  }

private:
  typedef std::map<uint32_t, std::shared_ptr<LinuxUinput> > UInputDevs;
  UInputDevs m_uinput_devs;

  typedef std::map<uint32_t, std::string> DeviceNames;
  DeviceNames m_device_names;

  typedef std::map<uint32_t, struct input_id> DeviceUSBId;
  DeviceUSBId m_device_usbids;

  typedef std::vector<UIEventCollectorPtr> Collectors;
  Collectors m_collectors;

  struct RelRepeat
  {
    UIEvent code;
    float value;
    float rest;
    int time_count;
    int repeat_interval;
  };

  std::map<UIEvent, RelRepeat> m_rel_repeat_lst;

  bool m_extra_events;

  guint m_timeout_id;
  GTimer* m_timer;

public:
  UInput(bool extra_events);
  ~UInput();

  /** guess the number of the next unused /dev/input/jsX device */
  static int  find_jsdev_number();

  /** guess the number of the next unused /dev/input/eventX device */
  static int  find_evdev_number();

  void set_device_names(const std::map<uint32_t, std::string>& device_names);
  void set_device_usbids(const std::map<uint32_t, struct input_id>& device_usbids);
  void set_ff_callback(int device_id, const boost::function<void (uint8_t, uint8_t)>& callback);

  /** Device construction functions
      @{*/
  UIEventEmitterPtr add(const UIEvent& ev);
  UIEventEmitterPtr add_rel(uint32_t device_id, int ev_code);
  UIEventEmitterPtr add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat);
  UIEventEmitterPtr add_key(uint32_t device_id, int ev_code);
  void add_ff(uint32_t device_id, uint16_t code);

  /** needs to be called to finish device creation and create the
      device in the kernel */
  void finish();
  /** @} */

  /** Send events to the kernel
      @{*/
  void send(uint32_t device_id, int ev_type, int ev_code, int value);
  void send_rel_repetitive(const UIEvent& code, float value, int repeat_interval);

  /** should be called to signal that all events of the current frame
      have been send */
  void sync();
  /** @} */

private:
  void update(int msec_delta);

  /** create a LinuxUinput with the given device_id, if some already
      exist return a pointer to it */
  LinuxUinput* create_uinput_device(uint32_t device_id);

  /** must only be called with a valid device_id */
  LinuxUinput* get_uinput(uint32_t device_id) const;

  std::string get_device_name(uint32_t device_id) const;
  struct input_id get_device_usbid(uint32_t device_id) const;

  bool on_timeout();
  static gboolean on_timeout_wrap(gpointer data) {
    return static_cast<UInput*>(data)->on_timeout();
  }

  UIEventEmitterPtr create_emitter(int device_id, int type, int code);

private:
  UInput(const UInput&);
  UInput& operator=(const UInput&);
};

#endif

/* EOF */
