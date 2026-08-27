#ifndef HEADER_XBOXDRV_HEADSET_PIPEWIRE_HPP
#define HEADER_XBOXDRV_HEADSET_PIPEWIRE_HPP

#include <cstddef>
#include <cstdint>
#include <memory>

namespace xboxdrv {

/**
 * Native PipeWire virtual devices for the Xbox 360 headset.
 *
 *   xboxdrv-headset-mic     — Audio/Source, 16 kHz mono S16LE (USB mic → graph)
 *   xboxdrv-headset-speaker — Audio/Sink,   48 kHz mono S16LE (graph → USB @ 8 kHz)
 *
 * Requires HAVE_PIPEWIRE (libpipewire-0.3). Used by --headset-pipewire.
 */
class HeadsetPipeWire
{
public:
  HeadsetPipeWire();
  ~HeadsetPipeWire();

  HeadsetPipeWire(const HeadsetPipeWire&) = delete;
  HeadsetPipeWire& operator=(const HeadsetPipeWire&) = delete;

  void start();
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
