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

/**
 * PipeWire's graph is built around ~48 kHz quanta. Advertising 8 kHz / 16 kHz
 * endpoints forces continuous non-integer-friendly conversion in the adapter
 * and is a common source of stutter. We run both nodes at 48 kHz and convert
 * to/from the headset rates with exact integer ratios:
 *   mic  16 kHz → 48 kHz  (×3)
 *   spk  48 kHz →  8 kHz  (÷6)
 */
constexpr uint32_t PW_RATE = 48000;
constexpr uint32_t MIC_USB_RATE = 16000;
constexpr uint32_t SPK_USB_RATE = 8000;
constexpr uint32_t MIC_UPSAMPLE = PW_RATE / MIC_USB_RATE; // 3
constexpr uint32_t SPK_DOWNSAMPLE = PW_RATE / SPK_USB_RATE; // 6

constexpr size_t MIC_RING = PW_RATE / 2;  // 500 ms @ 48 kHz
constexpr size_t SPK_RING = PW_RATE / 2;
constexpr size_t SPK_HIGH = PW_RATE / 2;     // only drop when ~500 ms deep
constexpr size_t SPK_TARGET = PW_RATE / 10;  // leave ~100 ms

class SampleRing
{
public:
  explicit SampleRing(size_t capacity) :
    m_buf(capacity), m_cap(capacity), m_r(0), m_w(0) {}

  size_t available() const
  {
    size_t r = m_r.load(std::memory_order_acquire);
    size_t w = m_w.load(std::memory_order_acquire);
    return (w - r + m_cap) % m_cap;
  }

  void discard(size_t n)
  {
    size_t r = m_r.load(std::memory_order_relaxed);
    size_t w = m_w.load(std::memory_order_acquire);
    size_t avail = (w - r + m_cap) % m_cap;
    m_r.store(r + std::min(n, avail), std::memory_order_release);
  }

  void clear()
  {
    m_r.store(m_w.load(std::memory_order_relaxed), std::memory_order_release);
  }

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
        discard(std::max<size_t>(1, m_cap / 8));
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
  SampleRing mic_ring{MIC_RING}; // 48 kHz samples
  SampleRing spk_ring{SPK_RING}; // 48 kHz samples
  std::atomic<bool> stopping{false};
  uint32_t mic_stride = sizeof(int16_t);
  uint32_t spk_stride = sizeof(int16_t);
  int16_t spk_last = 0;

  static void on_mic_process(void* data);
  static void on_spk_process(void* data);
  static void on_mic_state(void* data, pw_stream_state old, pw_stream_state state, const char* error);
  static void on_spk_state(void* data, pw_stream_state old, pw_stream_state state, const char* error);
  static void on_mic_param(void* data, uint32_t id, const spa_pod* param);
  static void on_spk_param(void* data, uint32_t id, const spa_pod* param);
  static void finish_buffers(pw_stream* stream, uint32_t stride, uint32_t rate);
};

void HeadsetPipeWire::Impl::finish_buffers(pw_stream* stream, uint32_t stride, uint32_t rate)
{
  uint8_t buffer[1024];
  spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const spa_pod* params[1];
  int32_t s = static_cast<int32_t>(stride);
  int32_t size = s * static_cast<int32_t>(std::max(32u, rate / 50)); // ~20 ms
  params[0] = static_cast<const spa_pod*>(spa_pod_builder_add_object(&b,
      SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
      SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(8, 2, 16),
      SPA_PARAM_BUFFERS_blocks,  SPA_POD_Int(1),
      SPA_PARAM_BUFFERS_size,    SPA_POD_CHOICE_RANGE_Int(size, s * 64, s * 8192),
      SPA_PARAM_BUFFERS_stride,  SPA_POD_Int(s)));
  pw_stream_update_params(stream, params, 1);
}

void HeadsetPipeWire::Impl::on_mic_state(void* /*data*/, pw_stream_state /*old*/,
                                         pw_stream_state state, const char* error)
{
  log_info("[headset] pipewire mic state: {}{}",
           pw_stream_state_as_string(state),
           error ? std::string(" — ") + error : std::string());
}

void HeadsetPipeWire::Impl::on_spk_state(void* /*data*/, pw_stream_state /*old*/,
                                         pw_stream_state state, const char* error)
{
  log_info("[headset] pipewire speaker state: {}{}",
           pw_stream_state_as_string(state),
           error ? std::string(" — ") + error : std::string());
}

void HeadsetPipeWire::Impl::on_mic_param(void* data, uint32_t id, const spa_pod* param)
{
  auto* self = static_cast<Impl*>(data);
  if (param == nullptr || id != SPA_PARAM_Format)
  {
    return;
  }
  spa_audio_info info{};
  if (spa_format_parse(param, &info.media_type, &info.media_subtype) < 0)
  {
    return;
  }
  if (info.media_type != SPA_MEDIA_TYPE_audio ||
      info.media_subtype != SPA_MEDIA_SUBTYPE_raw)
  {
    return;
  }
  spa_format_audio_raw_parse(param, &info.info.raw);
  self->mic_stride = sizeof(int16_t) * std::max(1u, info.info.raw.channels);
  log_info("[headset] pipewire mic format: rate={} channels={}",
           info.info.raw.rate, info.info.raw.channels);
  finish_buffers(self->mic_stream, self->mic_stride,
                 info.info.raw.rate ? info.info.raw.rate : PW_RATE);
}

void HeadsetPipeWire::Impl::on_spk_param(void* data, uint32_t id, const spa_pod* param)
{
  auto* self = static_cast<Impl*>(data);
  if (param == nullptr || id != SPA_PARAM_Format)
  {
    return;
  }
  spa_audio_info info{};
  if (spa_format_parse(param, &info.media_type, &info.media_subtype) < 0)
  {
    return;
  }
  if (info.media_type != SPA_MEDIA_TYPE_audio ||
      info.media_subtype != SPA_MEDIA_SUBTYPE_raw)
  {
    return;
  }
  spa_format_audio_raw_parse(param, &info.info.raw);
  self->spk_stride = sizeof(int16_t) * std::max(1u, info.info.raw.channels);
  log_info("[headset] pipewire speaker format: rate={} channels={}",
           info.info.raw.rate, info.info.raw.channels);
  finish_buffers(self->spk_stream, self->spk_stride,
                 info.info.raw.rate ? info.info.raw.rate : PW_RATE);
}

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
  uint32_t n_bytes = d->maxsize;
  if (b->requested > 0)
  {
    n_bytes = std::min(d->maxsize,
                       static_cast<uint32_t>(b->requested * self->mic_stride));
  }
  size_t n_samp = n_bytes / self->mic_stride;
  if (n_samp == 0)
  {
    pw_stream_queue_buffer(self->mic_stream, b);
    return;
  }

  auto* out = static_cast<int16_t*>(d->data);
  size_t got = self->mic_ring.read(out, n_samp);
  if (got < n_samp)
  {
    std::memset(out + got, 0, (n_samp - got) * sizeof(int16_t));
  }
  d->chunk->offset = 0;
  d->chunk->stride = static_cast<int32_t>(self->mic_stride);
  d->chunk->size = static_cast<uint32_t>(n_samp * self->mic_stride);
  pw_stream_queue_buffer(self->mic_stream, b);
}

void HeadsetPipeWire::Impl::on_spk_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }

  while (pw_buffer* b = pw_stream_dequeue_buffer(self->spk_stream))
  {
    spa_data* d = &b->buffer->datas[0];
    if (d->data && d->chunk && d->chunk->size > 0)
    {
      size_t n = d->chunk->size / self->spk_stride;
      auto* samples = reinterpret_cast<const int16_t*>(
        static_cast<uint8_t*>(d->data) + d->chunk->offset);
      if (self->spk_stride == sizeof(int16_t))
      {
        self->spk_ring.write(samples, n);
      }
      else
      {
        uint32_t ch = self->spk_stride / sizeof(int16_t);
        for (size_t i = 0; i < n; ++i)
        {
          self->spk_ring.write(&samples[i * ch], 1);
        }
      }
    }
    pw_stream_queue_buffer(self->spk_stream, b);
  }
}

namespace {

void connect_device_stream(pw_stream* stream, pw_direction direction)
{
  uint8_t buffer[1024];
  spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const spa_pod* params[1];

  spa_audio_info_raw info = {};
  info.format = SPA_AUDIO_FORMAT_S16;
  info.channels = 1;
  info.rate = PW_RATE;
  params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

  int res = pw_stream_connect(
    stream, direction, PW_ID_ANY,
    static_cast<pw_stream_flags>(
      PW_STREAM_FLAG_MAP_BUFFERS |
      PW_STREAM_FLAG_RT_PROCESS),
    params, 1);
  if (res < 0)
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

void HeadsetPipeWire::shutdown()
{
  if (!m_impl)
  {
    return;
  }
  m_impl->stopping.store(true, std::memory_order_release);
  m_running = false;
  m_impl->mic_ring.clear();
  m_impl->spk_ring.clear();
  if (!m_impl->loop)
  {
    return;
  }

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
  log_info("[headset] pipewire nodes released");
}

void HeadsetPipeWire::start()
{
  if (m_running)
  {
    return;
  }
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

    static const pw_stream_events mic_events = {
      .version = PW_VERSION_STREAM_EVENTS,
      .state_changed = Impl::on_mic_state,
      .param_changed = Impl::on_mic_param,
      .process = Impl::on_mic_process,
    };
    static const pw_stream_events spk_events = {
      .version = PW_VERSION_STREAM_EVENTS,
      .state_changed = Impl::on_spk_state,
      .param_changed = Impl::on_spk_param,
      .process = Impl::on_spk_process,
    };

    {
      pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_CLASS, "Audio/Source",
        PW_KEY_MEDIA_ROLE, "Communication",
        PW_KEY_NODE_NAME, "xboxdrv-headset-mic",
        PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset microphone",
        PW_KEY_NODE_VIRTUAL, "true",
        "node.linger", "false",
        nullptr);
      m_impl->mic_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-mic", props);
      if (!m_impl->mic_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (mic) failed");
      }
      pw_stream_add_listener(m_impl->mic_stream, &m_impl->mic_listener, &mic_events, m_impl.get());
      connect_device_stream(m_impl->mic_stream, PW_DIRECTION_OUTPUT);
    }

    {
      pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        PW_KEY_MEDIA_CLASS, "Audio/Sink",
        PW_KEY_MEDIA_ROLE, "Communication",
        PW_KEY_NODE_NAME, "xboxdrv-headset-speaker",
        PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset speaker",
        PW_KEY_NODE_VIRTUAL, "true",
        "node.linger", "false",
        nullptr);
      m_impl->spk_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-speaker", props);
      if (!m_impl->spk_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (speaker) failed");
      }
      pw_stream_add_listener(m_impl->spk_stream, &m_impl->spk_listener, &spk_events, m_impl.get());
      connect_device_stream(m_impl->spk_stream, PW_DIRECTION_INPUT);
    }

    pw_thread_loop_unlock(m_impl->loop);
    m_running = true;
    log_info("[headset] pipewire @ {} Hz graph rate (USB mic {} kHz / spk {} kHz via integer resample)",
             PW_RATE, MIC_USB_RATE / 1000, SPK_USB_RATE / 1000);
  }
  catch (...)
  {
    shutdown();
    throw;
  }
}

void HeadsetPipeWire::push_mic(const int16_t* samples, size_t count)
{
  if (!m_impl || m_impl->stopping.load(std::memory_order_acquire))
  {
    return;
  }
  // 16 kHz → 48 kHz: repeat each sample ×3
  std::vector<int16_t> up(count * MIC_UPSAMPLE);
  for (size_t i = 0; i < count; ++i)
  {
    for (uint32_t k = 0; k < MIC_UPSAMPLE; ++k)
    {
      up[i * MIC_UPSAMPLE + k] = samples[i];
    }
  }
  m_impl->mic_ring.write(up.data(), up.size());
}

bool HeadsetPipeWire::pull_speaker(int16_t* out, size_t count)
{
  if (!m_impl || m_impl->stopping.load(std::memory_order_acquire))
  {
    std::memset(out, 0, count * sizeof(int16_t));
    return false;
  }

  // Need count * 6 samples at 48 kHz to produce count samples at 8 kHz.
  size_t need_48 = count * SPK_DOWNSAMPLE;
  size_t avail = m_impl->spk_ring.available();

  // Pull-based: if the ring is low, ask the graph for another cycle (docs:
  // non-driving streams may request a cycle via pw_stream_trigger_process).
  if (avail < need_48 * 2 && m_impl->loop && m_impl->spk_stream)
  {
    pw_thread_loop_lock(m_impl->loop);
    if (!m_impl->stopping.load(std::memory_order_acquire) && m_impl->spk_stream)
    {
      if (pw_stream_get_state(m_impl->spk_stream, nullptr) == PW_STREAM_STATE_STREAMING)
      {
        pw_stream_trigger_process(m_impl->spk_stream);
      }
    }
    pw_thread_loop_unlock(m_impl->loop);
    avail = m_impl->spk_ring.available();
  }

  if (avail > SPK_HIGH)
  {
    m_impl->spk_ring.discard(avail - SPK_TARGET);
  }

  // Read what we can at 48 kHz; hold last sample on shortfall (less harsh than zero).
  std::vector<int16_t> buf48(need_48);
  size_t got = m_impl->spk_ring.read(buf48.data(), need_48);
  for (size_t i = got; i < need_48; ++i)
  {
    buf48[i] = m_impl->spk_last;
  }

  for (size_t i = 0; i < count; ++i)
  {
    out[i] = buf48[i * SPK_DOWNSAMPLE];
  }
  if (count > 0)
  {
    m_impl->spk_last = out[count - 1];
  }
  return got > 0;
}

} // namespace xboxdrv

/* EOF */
