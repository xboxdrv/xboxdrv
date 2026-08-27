#ifndef HEADER_XBOXDRV_G72X_DECODER_HPP
#define HEADER_XBOXDRV_G72X_DECODER_HPP

extern "C" {
#include <g72x.h>
}

#include <cstdint>
#include <vector>

namespace xboxdrv {

/** G.726-32 / G.721 4-bit ADPCM for Xbox 360 wired headset.

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

/** Matching encoder: S16LE samples → packed G.726-32 bytes.
    Same state machine as G72xDecoder. Packing selectable for OUT experiments.
 */
class G72xEncoder
{
private:
  struct g72x_state m_state;

public:
  G72xEncoder() :
    m_state()
  {
    g72x_init_state(&m_state);
  }

  void reset()
  {
    g72x_init_state(&m_state);
  }

  /** Encode even number of S16LE samples to packed bytes (2 samples → 1 byte).
      left_pack: high nibble first (opposite of mic/right-packed).
      Odd trailing sample is dropped. */
  void encode(const int16_t* samples, size_t count, std::vector<uint8_t>& out,
              bool left_pack = false)
  {
    out.clear();
    out.reserve(count / 2);
    for (size_t i = 0; i + 1 < count; i += 2)
    {
      int code1 = g721_encoder(samples[i],     AUDIO_ENCODING_LINEAR, &m_state);
      int code2 = g721_encoder(samples[i + 1], AUDIO_ENCODING_LINEAR, &m_state);
      code1 &= 0x0f;
      code2 &= 0x0f;
      if (left_pack)
      {
        // high nibble first
        out.push_back(static_cast<uint8_t>((code1 << 4) | code2));
      }
      else
      {
        // Right-packed: low nibble first (matches mic decoder)
        out.push_back(static_cast<uint8_t>(code1 | (code2 << 4)));
      }
    }
  }
};

} // namespace xboxdrv

#endif

/* EOF */
