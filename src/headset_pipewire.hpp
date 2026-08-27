#ifndef HEADER_XBOXDRV_HEADSET_PIPEWIRE_HPP
#define HEADER_XBOXDRV_HEADSET_PIPEWIRE_HPP

#include <cstddef>
#include <cstdint>
#include <memory>

namespace xboxdrv {

/** PipeWire graph nodes for --headset-pulse (client-owned; removed on shutdown). */
class HeadsetPipeWire
{
public:
  HeadsetPipeWire();
  ~HeadsetPipeWire();

  HeadsetPipeWire(const HeadsetPipeWire&) = delete;
  HeadsetPipeWire& operator=(const HeadsetPipeWire&) = delete;

  void start();
  /** Idempotent: drop streams/core immediately so nodes do not linger. */
  void shutdown();

  void push_mic(const int16_t* samples, size_t count);
  bool pull_speaker(int16_t* out, size_t count);
  bool running() const { return m_running; }

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
  bool m_running = false;
};

} // namespace xboxdrv

#endif

/* EOF */
