#ifndef HEADER_XBOXDRV_G72X_DECODER_HPP
#define HEADER_XBOXDRV_G72X_DECODER_HPP

extern "C" {
#include <g72x.h>
}

#include <cstdint>
#include <vector>

namespace xboxdrv {

/** G.726-32 / G.721 4-bit ADPCM decoder for Xbox 360 wired headset mic.

    Confirmed against Sun decode-g72x: -64 (or -4) -R -l, play as S16LE @ 16 kHz.
    Right-packed: low nibble first, then high nibble. Continuous state across
    32-byte USB packets.
 */
class G72xDecoder
{
private:
  struct g72x_state m_state;

public:
  G72xDecoder() :
    m_state()
  {
    g72x_init_state(&m_state);
  }

  /** Decode packed G.726-32 bytes to S16LE samples (2 samples per input byte). */
  void decode(const uint8_t* data, int len, std::vector<int16_t>& out)
  {
    out.clear();
    out.reserve(static_cast<size_t>(len) * 2);
    for (int i = 0; i < len; ++i)
    {
      uint8_t b = data[i];
      // Right-packed (-R): low nibble first
      int code1 = b & 0x0f;
      int code2 = (b >> 4) & 0x0f;
      int sample1 = g721_decoder(code1, AUDIO_ENCODING_LINEAR, &m_state);
      int sample2 = g721_decoder(code2, AUDIO_ENCODING_LINEAR, &m_state);
      out.push_back(static_cast<int16_t>(sample1));
      out.push_back(static_cast<int16_t>(sample2));
    }
  }
};

} // namespace xboxdrv

#endif

/* EOF */
