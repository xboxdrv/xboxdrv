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

#ifndef HEADER_NPP_GLIB_NPUT_HPP
#define HEADER_NPP_GLIB_NPUT_HPP

#include <iostream>

#include <glib.h>

#include <uinpp/device.hpp>
#include <uinpp/multi_device.hpp>

namespace uinpp {

class GlibDevice;

/** Wrap Device for Glib */
class GlibDevice
{
public:
  GlibDevice(Device& device) :
    m_device(device),
    m_io_channel(),
    m_source_id()
  {
    // start g_io_channel
    m_io_channel = g_io_channel_unix_new(m_device.get_fd());

    // set encoding to binary
    GError* error = NULL;
    if (g_io_channel_set_encoding(m_io_channel, NULL, &error) != G_IO_STATUS_NORMAL)
    {
      std::cerr << "GlibLinuxUInput: " << error->message << std::endl;
      g_error_free(error);
    }

    g_io_channel_set_buffered(m_io_channel, false);

    m_source_id = g_io_add_watch(m_io_channel,
                                 static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                                 [](GIOChannel* source, GIOCondition condition, gpointer userdata) -> gboolean {
                                   static_cast<Device*>(userdata)->read();
                                   return TRUE;
                                 }, this);
  }

  ~GlibDevice()
  {
    g_source_remove(m_source_id);
  }

private:
  Device& m_device;
  GIOChannel* m_io_channel;
  guint m_source_id;

public:
  GlibDevice(GlibDevice const&) = delete;
  GlibDevice& operator=(GlibDevice const&) = delete;
};

/** Wrap UInput for use with Glib */
class GlibMultiDevice
{
public:
  GlibMultiDevice(MultiDevice& uinput) :
    m_uinput(uinput),
    m_timeout_id(),
    m_timer(g_timer_new())
  {
    // FIXME: hardcoded timeout is kind of evil
    // FIXME: would be nicer if MultiDevice didn't depend on glib
    m_timeout_id = g_timeout_add(
      10,
      [](gpointer data) -> gboolean {
        GlibMultiDevice* const self = static_cast<GlibMultiDevice*>(data);
        int const msec_delta = static_cast<int>(g_timer_elapsed(self->m_timer, NULL) * 1000.0f);
        g_timer_reset(self->m_timer);
        self->m_uinput.update(msec_delta);
        return true; // do not remove the callback
      }, this);

    for (auto& device : m_uinput.get_devices()) {
      m_devices.emplace_back(std::make_unique<GlibDevice>(*device));
    }
  }

  ~GlibMultiDevice()
  {
    m_devices.clear();

    g_source_remove(m_timeout_id);
    g_timer_destroy(m_timer);
  }

public:
  MultiDevice& m_uinput;
  guint m_timeout_id;
  GTimer* m_timer;

  std::vector<std::unique_ptr<GlibDevice>> m_devices;

public:
  GlibMultiDevice(GlibMultiDevice const&) = delete;
  GlibMultiDevice& operator=(GlibMultiDevice const&) = delete;
};

} //  namespace uinpp

#endif

/* EOF */
