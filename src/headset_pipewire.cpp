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
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include <logmich/log.hpp>

namespace xboxdrv {

namespace {

/**
 * Rates must match the USB / G.726 paths exactly:
 *   mic  IN  — G.726-32 decode → S16LE @ 16 kHz (same as --headset-pulse)
 *   spk  OUT — S16LE @ 8 kHz encode (USB phone)
 *
 * Speaker Sink runs at 48 kHz (graph-friendly) with exact ÷6 downsample to 8 kHz.
 * Mic Source stays native 16 kHz; PipeWire resamples for clients that need more.
 * Always upsampling mic into a 48 kHz ring while the stream can negotiate 16 kHz
 * made capture sound slow and heavily distorted.
 */
constexpr uint32_t SPK_GRAPH_RATE = 48000;
constexpr uint32_t MIC_RATE = 16000;
constexpr uint32_t SPK_USB_RATE = 8000;
constexpr uint32_t SPK_DOWNSAMPLE = SPK_GRAPH_RATE / SPK_USB_RATE; // 6
/** One USB speaker packet: 64 samples @ 8 kHz = 8 ms = 384 samples @ 48 kHz. */
constexpr uint32_t SPK_QUANTUM = 384;
/** One USB mic packet: 64 samples @ 16 kHz = 4 ms. */
constexpr uint32_t MIC_QUANTUM = 64;

constexpr size_t MIC_RING = MIC_RATE / 2;       // 500 ms @ 16 kHz
constexpr size_t SPK_RING = SPK_GRAPH_RATE / 2; // 500 ms @ 48 kHz
constexpr size_t SPK_HIGH = SPK_GRAPH_RATE / 4;
constexpr size_t SPK_TARGET = SPK_GRAPH_RATE / 10;
constexpr size_t SPK_PREBUFFER = SPK_QUANTUM * 2;
/** Serve mic only after a couple of USB packets so process never pads zeros. */
constexpr size_t MIC_PREBUFFER = MIC_QUANTUM * 3; // ~12 ms @ 16 kHz

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
        // Drop oldest data so the writer never blocks (USB / process must stay RT-friendly).
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
  spa_source* drive_timer = nullptr;
  SampleRing mic_ring{MIC_RING}; // 16 kHz samples
  SampleRing spk_ring{SPK_RING}; // 48 kHz samples
  std::atomic<bool> stopping{false};
  std::atomic<bool> mic_streaming{false};
  std::atomic<bool> spk_streaming{false};
  uint32_t mic_stride = sizeof(int16_t);
  uint32_t spk_stride = sizeof(int16_t);
  int16_t mic_last = 0;
  int16_t spk_last = 0;
  bool mic_primed = false;

  static void on_mic_process(void* data);
  static void on_spk_process(void* data);
  static void on_mic_state(void* data, pw_stream_state old, pw_stream_state state, const char* error);
  static void on_spk_state(void* data, pw_stream_state old, pw_stream_state state, const char* error);
  static void on_mic_param(void* data, uint32_t id, const spa_pod* param);
  static void on_spk_param(void* data, uint32_t id, const spa_pod* param);
  static void finish_buffers(pw_stream* stream, uint32_t stride, uint32_t period_samples);
  static void on_drive_timer(void* data, uint64_t expirations);
  static int invoke_triggers(struct spa_loop* loop, bool async, uint32_t seq,
                             const void* data, size_t size, void* user_data);

  void arm_drive_timer();
  void disarm_drive_timer();
  void maybe_update_timer();
  void request_cycles(bool mic, bool spk);
};

void HeadsetPipeWire::Impl::finish_buffers(pw_stream* stream, uint32_t stride, uint32_t period_samples)
{
  uint8_t buffer[1024];
  spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const spa_pod* params[1];
  int32_t s = static_cast<int32_t>(stride);
  int32_t size = s * static_cast<int32_t>(std::max(32u, period_samples));
  params[0] = static_cast<const spa_pod*>(spa_pod_builder_add_object(&b,
      SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
      SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(8, 2, 16),
      SPA_PARAM_BUFFERS_blocks,  SPA_POD_Int(1),
      SPA_PARAM_BUFFERS_size,    SPA_POD_CHOICE_RANGE_Int(size, s * 64, s * 8192),
      SPA_PARAM_BUFFERS_stride,  SPA_POD_Int(s)));
  pw_stream_update_params(stream, params, 1);
}

void HeadsetPipeWire::Impl::on_mic_state(void* data, pw_stream_state /*old*/,
                                         pw_stream_state state, const char* error)
{
  auto* self = static_cast<Impl*>(data);
  log_info("[headset] pipewire mic state: {}{}",
           pw_stream_state_as_string(state),
           error ? std::string(" — ") + error : std::string());

  const bool streaming = (state == PW_STREAM_STATE_STREAMING);
  self->mic_streaming.store(streaming, std::memory_order_release);
  self->maybe_update_timer();
  if (streaming)
  {
    self->request_cycles(true, false);
  }
}

void HeadsetPipeWire::Impl::on_spk_state(void* data, pw_stream_state /*old*/,
                                         pw_stream_state state, const char* error)
{
  auto* self = static_cast<Impl*>(data);
  log_info("[headset] pipewire speaker state: {}{}",
           pw_stream_state_as_string(state),
           error ? std::string(" — ") + error : std::string());

  const bool streaming = (state == PW_STREAM_STATE_STREAMING);
  self->spk_streaming.store(streaming, std::memory_order_release);
  self->maybe_update_timer();
  if (streaming)
  {
    self->request_cycles(false, true);
  }
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
  finish_buffers(self->mic_stream, self->mic_stride, MIC_QUANTUM);
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
  finish_buffers(self->spk_stream, self->spk_stride, SPK_QUANTUM);
}

void HeadsetPipeWire::Impl::on_mic_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }

  // USB is the mic clock: only emit as many samples as the ring actually holds.
  // The previous path drained every PW buffer and zero-padded shortfalls, which
  // showed up as periodic all-zero gaps in recordings.
  while (pw_buffer* b = pw_stream_dequeue_buffer(self->mic_stream))
  {
    if (!b->buffer || !b->buffer->datas[0].data)
    {
      pw_stream_queue_buffer(self->mic_stream, b);
      continue;
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
      continue;
    }

    const size_t avail = self->mic_ring.available();
    if (!self->mic_primed)
    {
      if (avail < MIC_PREBUFFER)
      {
        // Still gathering USB packets — do not invent silence for clients.
        d->chunk->offset = 0;
        d->chunk->stride = static_cast<int32_t>(self->mic_stride);
        d->chunk->size = 0;
        pw_stream_queue_buffer(self->mic_stream, b);
        break;
      }
      self->mic_primed = true;
    }

    if (avail == 0)
    {
      // No more real samples this cycle. Hold last sample for at most one
      // buffer so the client keeps a continuous stream without zero gaps,
      // then stop — further buffers wait for the next USB packet.
      auto* out = static_cast<int16_t*>(d->data);
      for (size_t i = 0; i < n_samp; ++i)
      {
        out[i] = self->mic_last;
      }
      d->chunk->offset = 0;
      d->chunk->stride = static_cast<int32_t>(self->mic_stride);
      d->chunk->size = static_cast<uint32_t>(n_samp * self->mic_stride);
      pw_stream_queue_buffer(self->mic_stream, b);
      break;
    }

    auto* out = static_cast<int16_t*>(d->data);
    size_t got = self->mic_ring.read(out, n_samp);
    if (got > 0)
    {
      self->mic_last = out[got - 1];
    }
    for (size_t i = got; i < n_samp; ++i)
    {
      out[i] = self->mic_last;
    }
    d->chunk->offset = 0;
    d->chunk->stride = static_cast<int32_t>(self->mic_stride);
    d->chunk->size = static_cast<uint32_t>(n_samp * self->mic_stride);
    pw_stream_queue_buffer(self->mic_stream, b);

    // Match USB packet cadence: one full period per cycle when the ring is thin.
    if (self->mic_ring.available() < MIC_QUANTUM)
    {
      break;
    }
  }
}

void HeadsetPipeWire::Impl::on_spk_process(void* data)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }

  // USB is the speaker clock (~8 ms / packet). Only pull from clients while the
  // ring is below target. Draining every buffer on every timer tick made the
  // graph consume audio (and linked video) several times real-time.
  while (self->spk_ring.available() < SPK_TARGET)
  {
    pw_buffer* b = pw_stream_dequeue_buffer(self->spk_stream);
    if (!b)
    {
      break;
    }
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

void HeadsetPipeWire::Impl::on_drive_timer(void* data, uint64_t /*expirations*/)
{
  auto* self = static_cast<Impl*>(data);
  if (self->stopping.load(std::memory_order_acquire))
  {
    return;
  }
  // Mic: only drive when the ring has real USB samples (never on empty → zeros).
  if (self->mic_stream && self->mic_streaming.load(std::memory_order_acquire) &&
      pw_stream_is_driving(self->mic_stream) &&
      self->mic_ring.available() >= MIC_QUANTUM)
  {
    pw_stream_trigger_process(self->mic_stream);
  }
  // Speaker: only pull from clients when the USB side has room in the ring.
  // Unconditional triggers drained the graph faster than real-time (video raced).
  if (self->spk_stream && self->spk_streaming.load(std::memory_order_acquire) &&
      pw_stream_is_driving(self->spk_stream) &&
      self->spk_ring.available() < SPK_TARGET)
  {
    pw_stream_trigger_process(self->spk_stream);
  }
}

int HeadsetPipeWire::Impl::invoke_triggers(struct spa_loop* /*loop*/, bool /*async*/,
                                           uint32_t /*seq*/, const void* data,
                                           size_t /*size*/, void* user_data)
{
  auto* self = static_cast<Impl*>(user_data);
  // data points to a uint32_t mask: bit0 = mic, bit1 = spk
  uint32_t mask = 3;
  if (data)
  {
    mask = *static_cast<const uint32_t*>(data);
  }
  if (self->stopping.load(std::memory_order_acquire))
  {
    return 0;
  }
  if ((mask & 1u) && self->mic_stream &&
      self->mic_streaming.load(std::memory_order_acquire) &&
      pw_stream_is_driving(self->mic_stream) &&
      self->mic_ring.available() >= MIC_QUANTUM)
  {
    pw_stream_trigger_process(self->mic_stream);
  }
  if ((mask & 2u) && self->spk_stream &&
      self->spk_streaming.load(std::memory_order_acquire) &&
      pw_stream_is_driving(self->spk_stream) &&
      self->spk_ring.available() < SPK_TARGET)
  {
    pw_stream_trigger_process(self->spk_stream);
  }
  return 0;
}

void HeadsetPipeWire::Impl::arm_drive_timer()
{
  if (!loop || drive_timer)
  {
    return;
  }
  // 8 ms ticks match one USB speaker packet; mic still gates on ring depth.
  // USB is the ultimate clock — the timer only tops up when rings need it.
  drive_timer = pw_loop_add_timer(pw_thread_loop_get_loop(loop), on_drive_timer, this);
  if (drive_timer)
  {
    timespec value{0, 8 * 1000 * 1000};
    timespec interval{0, 8 * 1000 * 1000};
    pw_loop_update_timer(pw_thread_loop_get_loop(loop), drive_timer, &value, &interval, false);
  }
}

void HeadsetPipeWire::Impl::disarm_drive_timer()
{
  if (!loop || !drive_timer)
  {
    return;
  }
  pw_loop_destroy_source(pw_thread_loop_get_loop(loop), drive_timer);
  drive_timer = nullptr;
}

void HeadsetPipeWire::Impl::maybe_update_timer()
{
  const bool need =
    mic_streaming.load(std::memory_order_acquire) ||
    spk_streaming.load(std::memory_order_acquire);
  if (need)
  {
    arm_drive_timer();
  }
  else
  {
    disarm_drive_timer();
  }
}

void HeadsetPipeWire::Impl::request_cycles(bool mic, bool spk)
{
  if (!loop || stopping.load(std::memory_order_acquire))
  {
    return;
  }
  // Never lock the thread loop from the USB / libusb callback path.
  uint32_t mask = (mic ? 1u : 0u) | (spk ? 2u : 0u);
  if (mask == 0)
  {
    return;
  }
  // async=true: never block the USB/libusb callback on the PW loop.
  pw_loop_invoke(pw_thread_loop_get_loop(loop), invoke_triggers,
                 0, &mask, sizeof(mask), true, this);
}

namespace {

void connect_device_stream(pw_stream* stream, pw_direction direction,
                           uint32_t rate, bool driver)
{
  uint8_t buffer[1024];
  spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const spa_pod* params[1];

  spa_audio_info_raw info = {};
  info.format = SPA_AUDIO_FORMAT_S16;
  info.channels = 1;
  info.rate = rate;
  params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

  int flags = PW_STREAM_FLAG_MAP_BUFFERS;
  if (driver)
  {
    // USB is the clock: we drive graph cycles so clients are served on our
    // schedule (not the other way around).
    flags |= PW_STREAM_FLAG_DRIVER;
  }
  // No RT_PROCESS: process only does lock-free ring I/O; timer/invoke run on
  // the loop thread.

  int res = pw_stream_connect(
    stream, direction, PW_ID_ANY,
    static_cast<pw_stream_flags>(flags),
    params, 1);
  if (res < 0)
  {
    throw std::runtime_error("pw_stream_connect failed");
  }
  pw_stream_set_active(stream, true);
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
  m_impl->mic_streaming.store(false, std::memory_order_release);
  m_impl->spk_streaming.store(false, std::memory_order_release);
  m_impl->mic_primed = false;
  m_impl->mic_last = 0;
  m_running = false;
  m_impl->mic_ring.clear();
  m_impl->spk_ring.clear();
  if (!m_impl->loop)
  {
    return;
  }

  pw_thread_loop_lock(m_impl->loop);
  m_impl->disarm_drive_timer();
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
  m_impl->mic_streaming.store(false, std::memory_order_release);
  m_impl->spk_streaming.store(false, std::memory_order_release);
  m_impl->mic_primed = false;
  m_impl->mic_last = 0;

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
      char latency[32];
      std::snprintf(latency, sizeof(latency), "%u/%u", MIC_QUANTUM, MIC_RATE);
      char quantum[16];
      std::snprintf(quantum, sizeof(quantum), "%u", MIC_QUANTUM);
      char rate[16];
      std::snprintf(rate, sizeof(rate), "%u", MIC_RATE);

      pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_CLASS, "Audio/Source",
        PW_KEY_MEDIA_ROLE, "Communication",
        PW_KEY_NODE_NAME, "xboxdrv-headset-mic",
        PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset microphone",
        PW_KEY_NODE_VIRTUAL, "true",
        PW_KEY_NODE_LATENCY, latency,
        "node.force-rate", rate,
        "node.force-quantum", quantum,
        "node.driver", "true",
        "node.linger", "false",
        nullptr);
      m_impl->mic_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-mic", props);
      if (!m_impl->mic_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (mic) failed");
      }
      pw_stream_add_listener(m_impl->mic_stream, &m_impl->mic_listener, &mic_events, m_impl.get());
      // Native 16 kHz DRIVER Source — matches USB decode and --headset-pulse.
      connect_device_stream(m_impl->mic_stream, PW_DIRECTION_OUTPUT, MIC_RATE, true);
    }

    {
      char latency[32];
      std::snprintf(latency, sizeof(latency), "%u/%u", SPK_QUANTUM, SPK_GRAPH_RATE);
      char quantum[16];
      std::snprintf(quantum, sizeof(quantum), "%u", SPK_QUANTUM);
      char rate[16];
      std::snprintf(rate, sizeof(rate), "%u", SPK_GRAPH_RATE);

      pw_properties* props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        PW_KEY_MEDIA_CLASS, "Audio/Sink",
        PW_KEY_MEDIA_ROLE, "Communication",
        PW_KEY_NODE_NAME, "xboxdrv-headset-speaker",
        PW_KEY_NODE_DESCRIPTION, "Xbox 360 headset speaker",
        PW_KEY_NODE_VIRTUAL, "true",
        PW_KEY_NODE_LATENCY, latency,
        "node.force-rate", rate,
        "node.force-quantum", quantum,
        "node.driver", "true",
        "node.linger", "false",
        nullptr);
      m_impl->spk_stream = pw_stream_new(m_impl->core, "xboxdrv-headset-speaker", props);
      if (!m_impl->spk_stream)
      {
        pw_thread_loop_unlock(m_impl->loop);
        throw std::runtime_error("pw_stream_new (speaker) failed");
      }
      pw_stream_add_listener(m_impl->spk_stream, &m_impl->spk_listener, &spk_events, m_impl.get());
      // DRIVER Sink at 48 kHz; USB path downsamples to 8 kHz.
      connect_device_stream(m_impl->spk_stream, PW_DIRECTION_INPUT, SPK_GRAPH_RATE, true);
    }

    pw_thread_loop_unlock(m_impl->loop);
    m_running = true;
    log_info("[headset] pipewire mic {} Hz quantum {} / spk {} Hz quantum {} (USB phone {} kHz)",
             MIC_RATE, MIC_QUANTUM, SPK_GRAPH_RATE, SPK_QUANTUM, SPK_USB_RATE / 1000);
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
  // Ring is native 16 kHz S16LE — same rate as G.726 decode / --headset-pulse.
  m_impl->mic_ring.write(samples, count);
  // USB produced mic data: ask the driver Source to push to any capture clients.
  m_impl->request_cycles(true, false);
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

  // If the ring is running low, ask the PW thread to run another driver cycle.
  // Async invoke only — never pw_thread_loop_lock from the USB callback path.
  if (avail < need_48 * 2)
  {
    m_impl->request_cycles(false, true);
    avail = m_impl->spk_ring.available();
  }

  // Soft start: avoid a burst of underrun hold-last before the first client data.
  if (avail < SPK_PREBUFFER)
  {
    std::memset(out, 0, count * sizeof(int16_t));
    return false;
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
