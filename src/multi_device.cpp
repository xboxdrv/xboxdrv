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

#include "multi_device.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include <logmich/log.hpp>

#include "parse.hpp"
#include "abs_event_collector.hpp"
#include "key_event_collector.hpp"
#include "rel_event_collector.hpp"

namespace uinpp {

VirtualDevice::VirtualDevice(MultiDevice& parent, uint32_t device_id) :
  m_parent(parent),
  m_device_id(device_id)
{
}

void
VirtualDevice::set_name(std::string_view name)
{
  m_parent.set_device_name(m_device_id, name);
}

void
VirtualDevice::set_phys(std::string_view phys)
{
  m_parent.set_device_phys(m_device_id, phys);
}

void
VirtualDevice::set_prop(int prop)
{
  m_parent.set_device_prop(m_device_id, prop);
}

void
VirtualDevice::set_usbid(uint16_t bustype, uint16_t vendor, uint16_t product, uint16_t version)
{
  m_parent.set_device_usbid(m_device_id, { bustype, vendor, product, version });
}

EventEmitter*
VirtualDevice::add_rel(int ev_code)
{
  return m_parent.add_rel(m_device_id, ev_code);
}

EventEmitter*
VirtualDevice::add_abs(int ev_code, int min, int max, int fuzz, int flat, int resolution)
{
  return m_parent.add_abs(m_device_id, ev_code, min, max, fuzz, flat, resolution);
}

EventEmitter*
VirtualDevice::add_key(int ev_code)
{
  return m_parent.add_key(m_device_id, ev_code);
}

MultiDevice::MultiDevice() :
  m_virtual_devices(),
  m_devices(),
  m_device_names(),
  m_device_usbids(),
  m_device_phys(),
  m_device_prop(),
  m_collectors(),
  m_rel_repeat_lst(),
  m_extra_events(true)
{
}

MultiDevice::~MultiDevice()
{
}

void
MultiDevice::set_extra_events(bool extra_events)
{
  m_extra_events = extra_events;
}

struct input_id
MultiDevice::get_device_usbid(uint32_t device_id) const
{
  uint16_t slot_id = get_slot_id(device_id);
  uint16_t type_id = get_type_id(device_id);

  auto it = m_device_usbids.find(device_id);
  if (it != m_device_usbids.end())
  {
    // found an exact match, return it
    return it->second;
  }
  else
  {
    it = m_device_usbids.find(create_device_id(slot_id, DEVICEID_AUTO));
    if (it != m_device_usbids.end())
    {
      return it->second;
    }
    else
    {
      it = m_device_usbids.find(create_device_id(SLOTID_AUTO, type_id));
      if (it != m_device_usbids.end())
      {
        return it->second;
      }
      else
      {
        struct input_id usbid;
        usbid.bustype = 0;
        usbid.vendor  = 0;
        usbid.product = 0;
        usbid.version = 0;
        return usbid;
      }
    }
  }
}

std::string
MultiDevice::get_device_name(uint32_t device_id) const
{
  uint16_t slot_id = get_slot_id(device_id);
  uint16_t type_id = get_type_id(device_id);

  auto it = m_device_names.find(device_id);
  if (it != m_device_names.end())
  {
    // found an exact match, return it
    return it->second;
  }
  else
  {
    it = m_device_names.find(create_device_id(slot_id, DEVICEID_AUTO));
    if (it != m_device_names.end())
    {
      // found a match for the slot, build a name and return it
      std::ostringstream str;
      str << it->second;
      switch(type_id)
      {
        case 0:
        case DEVICEID_JOYSTICK:
          break;

        case DEVICEID_MOUSE:
          str << " - Mouse Emulation";
          break;

        case DEVICEID_KEYBOARD:
          str << " - Keyboard Emulation";
          break;

        default:
          str << " - " << device_id+1;
          break;
      }
      return str.str();
    }
    else
    {
      it = m_device_names.find(create_device_id(SLOTID_AUTO, type_id));

      if (it != m_device_names.end())
      {
        // found match for the type, build name and return it
        std::ostringstream str;
        str << it->second;
        if (slot_id > 0)
        {
          str << " #" << (slot_id+1);
        }
        return str.str();
      }
      else
      {
        std::ostringstream str;

        it = m_device_names.find(create_device_id(SLOTID_AUTO, DEVICEID_AUTO));

        if (it != m_device_names.end())
        {
          str << it->second;
        }
        else
        {
          str << "Xbox Gamepad (userspace driver)";
        }

        switch(type_id)
        {
          case DEVICEID_JOYSTICK:
            break;

          case DEVICEID_MOUSE:
            str << " - Mouse Emulation";
            break;

          case DEVICEID_KEYBOARD:
            str << " - Keyboard Emulation";
            break;

          default:
            str << " - " << device_id+1;
            break;
        }
        if (slot_id > 0)
        {
          str << " #" << (slot_id+1);
        }
        return str.str();
      }
    }
  }
}

VirtualDevice*
MultiDevice::create_device(int slot, DeviceType type)
{
  uint32_t const device_id = create_device_id(static_cast<uint16_t>(slot), static_cast<uint16_t>(type));
  auto const it = m_virtual_devices.find(device_id);

  if (it != m_virtual_devices.end()) {
    return it->second.get();
  } else {
    auto vdev = std::make_unique<VirtualDevice>(*this, device_id);
    VirtualDevice* tmp = vdev.get();
    m_virtual_devices[device_id] = std::move(vdev);
    return tmp;
  }
}

Device*
MultiDevice::create_uinput_device(uint32_t device_id)
{
  // DEVICEID_AUTO should not happen at this point as the user should
  // have called resolve_device_id()
  assert(device_id != DEVICEID_AUTO);

  auto const it = m_devices.find(device_id);
  if (it != m_devices.end())
  {
    // device already exist, so return it
    return it->second.get();
  }
  else
  {
    log_debug("create device: {}", device_id);
    DeviceType device_type;

    if (!m_extra_events)
    {
      device_type = DeviceType::GENERIC;
    }
    else
    {
      switch (device_id)
      {
        case DEVICEID_JOYSTICK:
          device_type = DeviceType::JOYSTICK;
          break;

        case DEVICEID_MOUSE:
          device_type = DeviceType::MOUSE;
          break;

        case DEVICEID_KEYBOARD:
          device_type = DeviceType::KEYBOARD;
          break;

        default:
          device_type = DeviceType::GENERIC;
          break;
      }
    }

    std::string dev_name = get_device_name(device_id);
    auto dev = std::make_unique<Device>(device_type, dev_name, get_device_usbid(device_id));

    {
      auto prop_it = m_device_prop.find(device_id);
      if (prop_it != m_device_prop.end()) {
        dev->set_prop(prop_it->second);
      }
    }

    {
      auto phys_it = m_device_phys.find(device_id);
      if (phys_it != m_device_phys.end()) {
        dev->set_phys(phys_it->second);
      }
    }

    Device* dev_tmp = dev.get();
    m_devices[device_id] = std::move(dev);

    log_debug("created uinput device: {} - '{}`", device_id, dev_name);

    return dev_tmp;
  }
}

EventEmitter*
MultiDevice::add(Event const& ev)
{
  Device* dev = create_uinput_device(ev.get_device_id());

  switch(ev.get_type())
  {
    case EV_KEY:
      dev->add_key(static_cast<uint16_t>(ev.get_code()));
      break;

    case EV_REL:
      dev->add_rel(static_cast<uint16_t>(ev.get_code()));
      break;

    case EV_ABS:
      dev->add_abs(static_cast<uint16_t>(ev.get_code()),
                   0, 0, 0, 0 /* min, max, fuzz, flat */); // BROKEN
      break;
  }

  return create_emitter(ev.get_device_id(), ev.get_type(), ev.get_code());
}

EventEmitter*
MultiDevice::add_key(uint32_t device_id, int ev_code)
{
  Device* dev = create_uinput_device(device_id);
  dev->add_key(static_cast<uint16_t>(ev_code));

  return create_emitter(device_id, EV_KEY, ev_code);
}

EventEmitter*
MultiDevice::add_rel(uint32_t device_id, int ev_code)
{
  Device* dev = create_uinput_device(device_id);
  dev->add_rel(static_cast<uint16_t>(ev_code));

  return create_emitter(device_id, EV_REL, ev_code);
}

EventEmitter*
MultiDevice::add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat, int resolution)
{
  Device* dev = create_uinput_device(device_id);
  dev->add_abs(static_cast<uint16_t>(ev_code), min, max, fuzz, flat, resolution);

  return create_emitter(device_id, EV_ABS, ev_code);
}

void
MultiDevice::add_ff(uint32_t device_id, uint16_t code)
{
  Device* dev = create_uinput_device(device_id);
  dev->add_ff(code);
}

EventEmitter*
MultiDevice::create_emitter(int device_id, int type, int code)
{
  // search for an already existing emitter
  for(auto i = m_collectors.begin(); i != m_collectors.end(); ++i)
  {
    if (static_cast<int>((*i)->get_device_id()) == device_id &&
        (*i)->get_type() == type &&
        (*i)->get_code() == code)
    {
      return (*i)->create_emitter();
    }
  }

  // no emitter found, create a new one
  switch(type)
  {
    case EV_ABS:
      {
        m_collectors.push_back(std::make_unique<AbsEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    case EV_KEY:
      {
        m_collectors.push_back(std::make_unique<KeyEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    case EV_REL:
      {
        m_collectors.push_back(std::make_unique<RelEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    default:
      assert(false && "unknown type");
      return {};
  }
}

void
MultiDevice::finish()
{
  for(auto i = m_devices.begin(); i != m_devices.end(); ++i)
  {
    i->second->finish();
  }
}

std::vector<Device*>
MultiDevice::get_devices() const
{
  std::vector<Device*> result;
  for (auto& it : m_devices) {
    result.emplace_back(it.second.get());
  }
  return result;
}

void
MultiDevice::send(uint32_t device_id, int ev_type, int ev_code, int value)
{
  get_uinput(device_id)->send(static_cast<uint16_t>(ev_type), static_cast<uint16_t>(ev_code), value);
}

void
MultiDevice::update(int msec_delta)
{
  for(auto i = m_rel_repeat_lst.begin(); i != m_rel_repeat_lst.end(); ++i)
  {
    i->second.time_count += msec_delta;

    // FIXME: shouldn't send out events multiple times, but accumulate
    // them instead and send out only once
    while (i->second.time_count >= i->second.repeat_interval)
    {
      // value can be float, but be can only send out int, so keep
      // track of the rest we don't send
      int i_value = static_cast<int>(i->second.value + truncf(i->second.rest));
      i->second.rest -= truncf(i->second.rest);
      i->second.rest += i->second.value - truncf(i->second.value);

      get_uinput(i->second.code.get_device_id())->send(EV_REL, static_cast<uint16_t>(i->second.code.code), i_value);
      i->second.time_count -= i->second.repeat_interval;
    }
  }

  for(auto i = m_devices.begin(); i != m_devices.end(); ++i)
  {
    i->second->update(msec_delta);
  }
}

void
MultiDevice::sync()
{
  for(auto i = m_collectors.begin(); i != m_collectors.end(); ++i)
  {
    (*i)->sync();
  }

  for(auto i = m_devices.begin(); i != m_devices.end(); ++i)
  {
    i->second->sync();
  }
}

void
MultiDevice::send_rel_repetitive(Event const& code, float value, int repeat_interval)
{
  if (repeat_interval < 0)
  { // remove rel_repeats from list
    // FIXME: should send the last value still in the repeater
    m_rel_repeat_lst.erase(code);
    // no need to send a event for rel, as it defaults to 0 anyway
  }
  else
  { // add rel_repeats to list
    auto const it = m_rel_repeat_lst.find(code);

    if (it == m_rel_repeat_lst.end())
    {
      RelRepeat rel_rep;
      rel_rep.code  = code;
      rel_rep.value = value;
      rel_rep.rest  = 0.0f;
      rel_rep.time_count = 0;
      rel_rep.repeat_interval = repeat_interval;
      m_rel_repeat_lst.insert(std::pair<Event, RelRepeat>(code, rel_rep));

      // Send the event once
      get_uinput(code.get_device_id())->send(EV_REL, static_cast<uint16_t>(code.code), static_cast<int32_t>(value));
    }
    else
    {
      // FIXME: send old value, store new value for rest

      it->second.code  = code;
      it->second.value = value;
      // it->second.time_count = do not touch this
      it->second.repeat_interval = repeat_interval;
    }
  }
}

Device*
MultiDevice::get_uinput(uint32_t device_id) const
{
  auto const it = m_devices.find(device_id);
  if (it != m_devices.end())
  {
    return it->second.get();
  }
  else
  {
    assert(0);
    std::ostringstream str;
    str << "Couldn't find uinput device: " << device_id;
    throw std::runtime_error(str.str());
  }
}

void
MultiDevice::set_device_name(uint32_t device_id, std::string_view name)
{
  m_device_names[device_id] = name;
}

void
MultiDevice::set_device_phys(uint32_t device_id, std::string_view phys)
{
  m_device_phys[device_id] = phys;
}

void
MultiDevice::set_device_prop(uint32_t device_id, int prop)
{
  m_device_prop[device_id] = prop;
}

void
MultiDevice::set_device_usbid(uint32_t device_id, input_id id)
{
  m_device_usbids[device_id] = id;
}

void
MultiDevice::set_device_usbids(const std::map<uint32_t, struct input_id>& device_usbids)
{
  m_device_usbids = device_usbids;
}

void
MultiDevice::set_device_names(const std::map<uint32_t, std::string>& device_names)
{
  m_device_names = device_names;
}

void
MultiDevice::set_ff_callback(int device_id, const std::function<void (uint8_t, uint8_t)>& callback)
{
  get_uinput(device_id)->set_ff_callback(callback);
}

} // namespace uinpp

/* EOF */
