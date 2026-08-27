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

/** SPSC ring. On overflow, drops the oldest samples so latency cannot grow without bound. */
class SampleRing
{
public:
  explicit SampleRing(size_t capacity) :
    m_buf(capacity), m_cap(capacity), m_r(0), m_w(0) {}

  size_t capacity() const { return m_cap; }

  size_t available() const
  {
    size_t r = m_r.load(std::memory_order_acquire);
    size_t w = m_w.load(std::memory_order_acquire);
    return (w - r + m_cap) % m_cap;
  }

  /** Discard up to n oldest samples (used to catch up when lag builds). */
  void discard(size_t n)
  {
    size_t r = m_r.load(std::memory_order_relaxed);
    size_t w = m_w.load(std::memory_order_acquire);
    size_t avail = (w - r + m_cap) % m_cap;
    size_t drop = std::min(n, avail);
    m_r.store(r + drop, std::memory_order_release);
  }

  void clear()
  {
    size_t w = m_w.load(std::memory_order_relaxed);
    m_r.store(w, std::memory_order_release);
  }

  /** Write; if full, drop oldest to make room (prefer glitch over growing delay). */
  size_t write(const int16_t* data, size_t n)
  {
    size_t done = 0;
    while (done < n)
    {
      size_t w = m_w.load(std::memory_order_relaxed);
      size_t r = m_r.load(std::memory_order_acquire);
      size_t free = m_cap - 1 - ((w - r + m_cap) % m_cap);
      if (free == 0)
      {
        // Drop about 1/4 of the buffer of oldest audio to catch up.
        size_t drop = std::max<size_t>(1, m_cap / 4);
        discard(drop);
        continue;
      }
      size_t chunk = std::min(n - done, free);
      size_t pos = w % m_cap;
      size_t first = std::min(chunk, m_cap - pos);
      std::memcpy(m_buf.data() + pos, data + done, first * sizeof(int16_t));
      if (chunk > first)
      {
        std::memcpy(m_buf.data(), data + done + first, (chunk - first) * sizeof(int16_t));
      }
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
      if (avail == 0)
      {
        break;
      }
      size_t chunk = std::min(n - done, avail);
      size_t pos = r % m_cap;
      size_t first = std::min(chunk, m_cap - pos);
      std::memcpy(data + done, m_buf.data() + pos, first * sizeof(int16_t));
      if (chunk > first)
      {
        std::memcpy(data + done + first, m_buf.data(), (chunk - first) * sizeof(int16_t));
      }
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

// Keep only a few USB periods of slack (~100–200 ms), not a full second.
constexpr size_t MIC_RING_SAMPLES = 16000 / 5;   // ~200 ms @ 16 kHz
constexpr size_t SPK_RING_SAMPLES = 8000 / 5;    // ~200 ms @ 8 kHz
constexpr size_t SPK_HIGH_WATER   = 8000 / 10;   // ~100 ms — start dropping lag
constexpr size_t SPK_TARGET       = 8000 / 25;   // ~40 ms — target depth after catch-up

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
  SampleRing mic_ring{MIC_RING_SAMPLES};
  SampleRing spk_ring{SPK_RING_SAMPLES};
  std::atomic<bool> stopping{false};

  static void on_mic_process(void* data);
  static void on_spk_process(void* data);
};

void HeadsetPipeWire::Impl::on_mic_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }

  pw_buffer* b = pw_stream_dequeue_buffer(self->mic_stream);
  if (!b || !b->buffer || !b->buffer->datas[0].data)
  {
    if (b)
    {
      pw_stream_queue_buffer(self->mic_stream, b);
    }
    return;
  }
  spa_data* d = &b->buffer->datas[0];
  size_t max_samples = d->maxsize / sizeof(int16_t);
  // Prefer short quanta: only fill what we have, do not pad a whole large buffer
  // with silence (that invents time and desyncs the graph).
  size_t avail = self->mic_ring.available();
  size_t want = std::min(max_samples, std::max(avail, size_t(1)));
  // Cap single process fill so we do not dump the entire ring in one quantum.
  want = std::min(want, size_t(320)); // 20 ms @ 16 kHz
  size_t n = self->mic_ring.read(static_cast<int16_t*>(d->data), want);
  if (n == 0)
  {
    // True underrun: one silent quantum, size 1 period-ish
    n = std::min(max_samples, size_t(160));
    std::memset(d->data, 0, n * sizeof(int16_t));
  }
  d->chunk->offset = 0;
  d->chunk->stride = sizeof(int16_t);
  d->chunk->size = static_cast<uint32_t>(n * sizeof(int16_t));
  pw_stream_queue_buffer(self->mic_stream, b);
}

void HeadsetPipeWire::Impl::on_spk_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }

  pw_buffer* b = pw_stream_dequeue_buffer(self->spk_stream);
  if (!b)
  {
    return;
  }
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
  shutdown();
}

void
HeadsetPipeWire::shutdown()
{
  if (!m_impl)
  {
    return;
  }

  // Make process callbacks no-ops before we tear the graph down.
  m_impl->stopping.store(true, std::memory_order_release);
  m_running = false;
  m_impl->mic_ring.clear();
  m_impl->spk_ring.clear();

  if (!m_impl->loop)
  {
    return;
  }

  // Drop client objects while the loop is still running so PipeWire can
  // process destroy events; then disconnect the core (removes any leftover
  // client-owned nodes) and stop the thread.
  pw_thread_loop_lock(m_impl->loop);

  if (m_impl->mic_stream)
  {
    pw_stream_set_active(m_impl->mic_stream, false);
    pw_stream_disconnect(m_impl->mic_stream);
    pw_stream_destroy(m_impl->mic_stream);
    m_impl->mic_stream = nullptr;
  }
  if (m_impl->spk_stream)
  {
    pw_stream_set_active(m_impl->spk_stream, false);
    pw_stream_disconnect(m_impl->spk_stream);
    pw_stream_destroy(m_impl->spk_stream);
    m_impl->spk_stream = nullptr;
  }

  // Disconnecting the core is what guarantees the session drops our nodes
  // even if a stream destroy raced; do this before stopping the thread.
  if (m_impl->core)
  {
    pw_core_disconnect(m_impl->core);
    m_impl->core = nullptr;
  }
  if (m_impl->context)
  {
    pw_context_destroy(m_impl->context);
    m_impl->context = nullptr;
  }

  pw_thread_loop_unlock(m_impl->loop);
  pw_thread_loop_stop(m_impl->loop);
  pw_thread_loop_destroy(m_impl->loop);
  m_impl->loop = nullptr;

  pw_deinit();
  log_info("[headset] PipeWire nodes released");
}

void HeadsetPipeWire::start()
{
  if (m_running)
  {
    return;
  }

  // Clean up a previous partial start, if any.
  shutdown();
  m_impl->stopping.store(false, std::memory_order_release);

  pw_init(nullptr, nullptr);

  try
  {
    m_impl->loop = pw_thread_loop_new("xboxdrv-headset", nullptr);
    if (!m_impl->loop)
    {
      throw std::runtime_error("pw_thread_loop_new failed");
    }

    m_impl->context = pw_context_new(pw_thread_loop_get_loop(m_impl->loop), nullptr, 0);
    if (!m_impl->context)
    {
      throw std::runtime_error("pw_context_new failed");
    }

    if (pw_thread_loop_start(m_impl->loop) < 0)
    {
      throw std::runtime_error("pw_thread_loop_start failed");
    }

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
        PW_KEY_NODE_LATENCY, "256/16000",
        PW_KEY_NODE_WANT_DRIVER, "true",
        "node.linger", "false",
        nullptr);
      m_impl->mic_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-mic", props);
      if (!m_impl->mic_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (mic) failed");
      }
      static const pw_stream_events mic_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = Impl::on_mic_process,
      };
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
        PW_KEY_NODE_LATENCY, "128/8000",
        PW_KEY_NODE_WANT_DRIVER, "true",
        "node.linger", "false",
        nullptr);
      m_impl->spk_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-speaker", props);
      if (!m_impl->spk_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (speaker) failed");
      }
      static const pw_stream_events spk_events = {
        .version = PW_VERSION_STREAM_EVENTS,
        .process = Impl::on_spk_process,
      };
      pw_stream_add_listener(m_impl->spk_stream, &m_impl->spk_listener, &spk_events, m_impl.get());
      connect_audio_stream(m_impl->spk_stream, 8000, SPA_DIRECTION_INPUT);
    }

    pw_thread_loop_unlock(m_impl->loop);
    m_running = true;
    log_info("[headset] PipeWire: xboxdrv-headset-mic (16 kHz), "
             "xboxdrv-headset-speaker (8 kHz); nodes removed on exit");
  }
  catch (...)
  {
    // Drop any half-created client objects so we never leave orphan sinks.
    shutdown();
    throw;
  }
}

void HeadsetPipeWire::push_mic(const int16_t* samples, size_t count)
{
  if (m_impl && !m_impl->stopping.load(std::memory_order_acquire))
  {
    m_impl->mic_ring.write(samples, count);
  }
}

bool HeadsetPipeWire::pull_speaker(int16_t* out, size_t count)
{
  if (!m_impl || m_impl->stopping.load(std::memory_order_acquire))
  {
    std::memset(out, 0, count * sizeof(int16_t));
    return false;
  }

  // If PipeWire is slightly faster than USB, lag accumulates. Drop down to a
  // small target depth instead of playing farther and farther behind.
  size_t avail = m_impl->spk_ring.available();
  if (avail > SPK_HIGH_WATER)
  {
    m_impl->spk_ring.discard(avail - SPK_TARGET);
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
