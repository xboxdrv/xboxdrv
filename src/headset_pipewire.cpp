/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "headset_pipewire.hpp"

#include <atomic>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include <logmich/log.hpp>

namespace xboxdrv {

namespace {

class SampleRing
{
public:
  explicit SampleRing(size_t capacity) :
    m_buf(capacity), m_cap(capacity), m_r(0), m_w(0) {}

  size_t write(const int16_t* data, size_t n)
  {
    size_t done = 0;
    while (done < n)
    {
      size_t w = m_w.load(std::memory_order_relaxed);
      size_t r = m_r.load(std::memory_order_acquire);
      size_t free = m_cap - 1 - ((w - r + m_cap) % m_cap);
      if (free == 0) break;
      size_t chunk = std::min(n - done, free);
      size_t pos = w % m_cap;
      size_t first = std::min(chunk, m_cap - pos);
      std::memcpy(m_buf.data() + pos, data + done, first * sizeof(int16_t));
      if (chunk > first)
        std::memcpy(m_buf.data(), data + done + first, (chunk - first) * sizeof(int16_t));
      m_w.store(w + chunk, std::memory_order_release);
      done += chunk;
    }
    return done;
  }

  size_t read(int16_t* data, size_t n)
  {
    size_t done = 0;
    while (done < n)
    {
      size_t r = m_r.load(std::memory_order_relaxed);
      size_t w = m_w.load(std::memory_order_acquire);
      size_t avail = (w - r + m_cap) % m_cap;
      if (avail == 0) break;
      size_t chunk = std::min(n - done, avail);
      size_t pos = r % m_cap;
      size_t first = std::min(chunk, m_cap - pos);
      std::memcpy(data + done, m_buf.data() + pos, first * sizeof(int16_t));
      if (chunk > first)
        std::memcpy(data + done + first, m_buf.data(), (chunk - first) * sizeof(int16_t));
      m_r.store(r + chunk, std::memory_order_release);
      done += chunk;
    }
    return done;
  }

private:
  std::vector<int16_t> m_buf;
  size_t m_cap;
  std::atomic<size_t> m_r, m_w;
};

} // namespace

struct HeadsetPipeWire::Impl
{
  pw_thread_loop* loop = nullptr;
  pw_context* context = nullptr;
  pw_core* core = nullptr;
  pw_stream* mic_stream = nullptr;
  pw_stream* spk_stream = nullptr;
  spa_hook mic_listener{};
  spa_hook spk_listener{};
  SampleRing mic_ring{16000};
  SampleRing spk_ring{8000};

  static void on_mic_process(void* data);
  static void on_spk_process(void* data);
};

void HeadsetPipeWire::Impl::on_mic_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  pw_buffer* b = pw_stream_dequeue_buffer(self->mic_stream);
  if (!b || !b->buffer || !b->buffer->datas[0].data)
  {
    if (b) pw_stream_queue_buffer(self->mic_stream, b);
    return;
  }
  spa_data* d = &b->buffer->datas[0];
  size_t max_samples = d->maxsize / sizeof(int16_t);
  size_t n = self->mic_ring.read(static_cast<int16_t*>(d->data), max_samples);
  if (n < max_samples)
  {
    std::memset(static_cast<int16_t*>(d->data) + n, 0, (max_samples - n) * sizeof(int16_t));
    n = max_samples;
  }
  d->chunk->offset = 0;
  d->chunk->stride = sizeof(int16_t);
  d->chunk->size = static_cast<uint32_t>(n * sizeof(int16_t));
  pw_stream_queue_buffer(self->mic_stream, b);
}

void HeadsetPipeWire::Impl::on_spk_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  pw_buffer* b = pw_stream_dequeue_buffer(self->spk_stream);
  if (!b) return;
  spa_data* d = &b->buffer->datas[0];
  if (d->data && d->chunk && d->chunk->size > 0)
  {
    size_t n = d->chunk->size / sizeof(int16_t);
    auto* samples = reinterpret_cast<const int16_t*>(
      static_cast<uint8_t*>(d->data) + d->chunk->offset);
    self->spk_ring.write(samples, n);
  }
  pw_stream_queue_buffer(self->spk_stream, b);
}

namespace {

const pw_stream_events mic_events = {
  .version = PW_VERSION_STREAM_EVENTS,
  .process = HeadsetPipeWire::Impl::on_mic_process,
};

const pw_stream_events spk_events = {
  .version = PW_VERSION_STREAM_EVENTS,
  .process = HeadsetPipeWire::Impl::on_spk_process,
};

void connect_audio_stream(pw_stream* stream, uint32_t rate, spa_direction direction)
{
  uint8_t buffer[1024];
  spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const spa_pod* params[1];
  spa_audio_info_raw info = {};
  info.format = SPA_AUDIO_FORMAT_S16;
  info.channels = 1;
  info.rate = rate;
  params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

  if (pw_stream_connect(stream, direction, PW_ID_ANY,
                        static_cast<pw_stream_flags>(
                          PW_STREAM_FLAG_AUTOCONNECT |
                          PW_STREAM_FLAG_MAP_BUFFERS |
                          PW_STREAM_FLAG_RT_PROCESS),
                        params, 1) < 0)
  {
    throw std::runtime_error("pw_stream_connect failed");
  }
}

} // namespace

HeadsetPipeWire::HeadsetPipeWire() :
  m_impl(std::make_unique<Impl>()),
  m_running(false)
{
}

HeadsetPipeWire::~HeadsetPipeWire()
{
  if (!m_impl || !m_impl->loop) return;

  pw_thread_loop_lock(m_impl->loop);
  if (m_impl->mic_stream) { pw_stream_destroy(m_impl->mic_stream); m_impl->mic_stream = nullptr; }
  if (m_impl->spk_stream) { pw_stream_destroy(m_impl->spk_stream); m_impl->spk_stream = nullptr; }
  if (m_impl->core) { pw_core_disconnect(m_impl->core); m_impl->core = nullptr; }
  if (m_impl->context) { pw_context_destroy(m_impl->context); m_impl->context = nullptr; }
  pw_thread_loop_unlock(m_impl->loop);
  pw_thread_loop_stop(m_impl->loop);
  pw_thread_loop_destroy(m_impl->loop);
  m_impl->loop = nullptr;
  pw_deinit();
  m_running = false;
}

void HeadsetPipeWire::start()
{
  pw_init(nullptr, nullptr);

  m_impl->loop = pw_thread_loop_new("xboxdrv-headset", nullptr);
  if (!m_impl->loop) throw std::runtime_error("pw_thread_loop_new failed");

  m_impl->context = pw_context_new(pw_thread_loop_get_loop(m_impl->loop), nullptr, 0);
  if (!m_impl->context) throw std::runtime_error("pw_context_new failed");

  if (pw_thread_loop_start(m_impl->loop) < 0)
    throw std::runtime_error("pw_thread_loop_start failed");

  pw_thread_loop_lock(m_impl->loop);
  m_impl->core = pw_context_connect(m_impl->context, nullptr, 0);
  if (!m_impl->core)
  {
    pw_thread_loop_unlock(m_impl->loop);
    throw std::runtime_error("pw_context_connect failed (is PipeWire running?)");
  }

  {
    pw_properties* props = pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Audio",
      PW_KEY_MEDIA_CATEGORY, "Capture",
      PW_KEY_MEDIA_ROLE, "Communication",
      PW_KEY_NODE_NAME, "xboxdrv-headset-mic",
      PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset microphone",
      nullptr);
    m_impl->mic_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-mic", props);
    if (!m_impl->mic_stream)
    {
      pw_thread_loop_unlock(m_impl->loop);
      throw std::runtime_error("pw_stream_new (mic) failed");
    }
    pw_stream_add_listener(m_impl->mic_stream, &m_impl->mic_listener, &mic_events, m_impl.get());
    connect_audio_stream(m_impl->mic_stream, 16000, SPA_DIRECTION_OUTPUT);
  }

  {
    pw_properties* props = pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Audio",
      PW_KEY_MEDIA_CATEGORY, "Playback",
      PW_KEY_MEDIA_ROLE, "Communication",
      PW_KEY_NODE_NAME, "xboxdrv-headset-speaker",
      PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset speaker",
      nullptr);
    m_impl->spk_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-speaker", props);
    if (!m_impl->spk_stream)
    {
      pw_thread_loop_unlock(m_impl->loop);
      throw std::runtime_error("pw_stream_new (speaker) failed");
    }
    pw_stream_add_listener(m_impl->spk_stream, &m_impl->spk_listener, &spk_events, m_impl.get());
    connect_audio_stream(m_impl->spk_stream, 8000, SPA_DIRECTION_INPUT);
  }

  pw_thread_loop_unlock(m_impl->loop);
  m_running = true;
  log_info("[headset] PipeWire: xboxdrv-headset-mic (16 kHz source), "
           "xboxdrv-headset-speaker (8 kHz sink)");
}

void HeadsetPipeWire::push_mic(const int16_t* samples, size_t count)
{
  if (m_impl) m_impl->mic_ring.write(samples, count);
}

bool HeadsetPipeWire::pull_speaker(int16_t* out, size_t count)
{
  if (!m_impl)
  {
    std::memset(out, 0, count * sizeof(int16_t));
    return false;
  }
  size_t n = m_impl->spk_ring.read(out, count);
  if (n < count)
  {
    std::memset(out + n, 0, (count - n) * sizeof(int16_t));
    return n > 0;
  }
  return true;
}

} // namespace xboxdrv

/* EOF */
