#ifndef HEADER_XBOXDRV_G72X_DECODER_HPP
#define HEADER_XBOXDRV_G72X_DECODER_HPP

extern "C" {
#include <g72x.h>
}

#include <vector>
#include <cstdint>

namespace xboxdrv {

class G72xDecoder
{
private:
  struct g72x_state m_state;

public:
  G72xDecoder()
  {
    g72x_init_state(&m_state);
  }

  /** Decode 32-byte G.721 packet (64 nibbles) to 64 S16 samples. */
  void decode(const uint8_t* data, int len, std::vector<int16_t>& out)
  {
    out.clear();
    out.reserve(len * 2);
    for (int i = 0; i < len; ++i)
    {
      uint8_t b = data[i];
      // high nibble first? or low? try both later; common is low first for packing
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
