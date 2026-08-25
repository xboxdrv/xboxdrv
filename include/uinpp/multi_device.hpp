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

#ifndef HEADER_NPP_MULTI_DEVICE_HPP
#define HEADER_NPP_MULTI_DEVICE_HPP

#include <map>
#include <string_view>

#include "fwd.hpp"
#include "device.hpp"
#include "event.hpp"

namespace uinpp {

class VirtualDevice
{
public:
  VirtualDevice(MultiDevice& parent, uint32_t device_id);

  void set_name(std::string_view name);
  void set_phys(std::string_view phys);
  void set_prop(int prop);
  void set_usbid(uint16_t bustype, uint16_t vendor, uint16_t product, uint16_t version);

  EventEmitter* add_rel(int ev_code);
  EventEmitter* add_abs(int ev_code, int min, int max, int fuzz, int flat, int resolution);
  EventEmitter* add_key(int ev_code);

private:
  MultiDevice& m_parent;
  uint32_t m_device_id;
};

/** MultiDevice bundle multiple devices to make it easier to create
    virtual devices that spread across different categories of input, e.g. a
    keyboard with a trackball would both need a mouse device as well as
    a keyboard one */
class MultiDevice
{
public:
  MultiDevice();
  ~MultiDevice();

  /** Device construction functions
      @{*/
  /** Create the usual events that would be present for a device of the given type automatically */
  void set_extra_events(bool extra_events);

  void set_device_names(std::map<uint32_t, std::string> const& device_names);
  void set_device_usbids(std::map<uint32_t, struct input_id> const& device_usbids);

  void set_device_name(uint32_t device_id, std::string_view name);
  void set_device_phys(uint32_t device_id, std::string_view phys);
  void set_device_prop(uint32_t device_id, int prop);
  void set_device_usbid(uint32_t device_id, input_id id);

  void set_ff_callback(int device_id, std::function<void (uint8_t, uint8_t)> const& callback);

  VirtualDevice* create_device(int slot, DeviceType type);

  EventEmitter* add(Event const& ev);
  EventEmitter* add_rel(uint32_t device_id, int ev_code);
  EventEmitter* add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat, int resolution);
  EventEmitter* add_key(uint32_t device_id, int ev_code);

  void add_ff(uint32_t device_id, uint16_t code);

  /** needs to be called to finish device creation and create the
      device in the kernel */
  void finish();
  /** @} */

  /** Send events directly to the kernel
      @{*/
  void send(uint32_t device_id, int ev_type, int ev_code, int value);
  void send_rel_repetitive(Event const& code, float value, int repeat_interval);

  /** should be called to signal that all events of the current frame
      have been send */
  void sync();
  /** @} */

  std::vector<Device*> get_devices() const;

  void update(int msec_delta);

private:
  /** create a Device with the given device_id, if some already
      exist return a pointer to it */
  Device* create_uinput_device(uint32_t device_id);

  /** must only be called with a valid device_id */
  Device* get_uinput(uint32_t device_id) const;

  std::string get_device_name(uint32_t device_id) const;
  struct input_id get_device_usbid(uint32_t device_id) const;

  EventEmitter* create_emitter(int device_id, int type, int code);

private:
  struct RelRepeat
  {
    Event code;
    float value;
    float rest;
    int time_count;
    int repeat_interval;
  };

private:
  std::map<uint32_t, std::unique_ptr<VirtualDevice> > m_virtual_devices;
  std::map<uint32_t, std::unique_ptr<Device> > m_devices;
  std::map<uint32_t, std::string> m_device_names;
  std::map<uint32_t, struct input_id> m_device_usbids;
  std::map<uint32_t, std::string> m_device_phys;
  std::map<uint32_t, int> m_device_prop;
  std::vector<std::unique_ptr<EventCollector>> m_collectors;

  std::map<Event, RelRepeat> m_rel_repeat_lst;

  bool m_extra_events;

private:
  MultiDevice(MultiDevice const&);
  MultiDevice& operator=(MultiDevice const&);
};

} // namespace uinpp

#endif

/* EOF */
