/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "headset.hpp"

#include <fstream>
#include <functional>
#include <errno.h>
#include <string.h>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include <unsebu/usb_helper.hpp>

#include "raise_exception.hpp"
#include "util/string.hpp"
#include <logmich/log.hpp>

namespace xboxdrv {

using namespace std::placeholders;

namespace {

/** Sample rate for --headset-dump-wav / mic decode (matches working capture path). */
constexpr uint32_t WAV_SAMPLE_RATE = 16000;
/** Playback target rate: 360 headset render is ~8 kHz (One is 24 kHz = 3×). */
constexpr uint32_t PLAY_SAMPLE_RATE = 8000;
constexpr uint16_t WAV_CHANNELS = 1;
constexpr uint16_t WAV_BITS = 16;
/** Samples per 32-byte G.726-32 packet (2 samples per byte). */
constexpr size_t SAMPLES_PER_PACKET = 64;

uint16_t read_u16_le(const char* p)
{
  return static_cast<uint16_t>(static_cast<uint8_t>(p[0]) |
                               (static_cast<uint8_t>(p[1]) << 8));
}

uint32_t read_u32_le(const char* p)
{
  return static_cast<uint32_t>(static_cast<uint8_t>(p[0]) |
                               (static_cast<uint8_t>(p[1]) << 8) |
                               (static_cast<uint8_t>(p[2]) << 16) |
                               (static_cast<uint8_t>(p[3]) << 24));
}

} // namespace

Headset::Headset(libusb_device_handle* handle, bool debug) :
  m_handle(handle),
  m_interface(new unsebu::USBInterface(m_handle, 1)),
  m_fout_raw(),
  m_fout_pcm(),
  m_fout_wav(),
  m_wav_data_bytes(0),
  m_fin(),
  m_play_pcm(),
  m_play_pos(0),
  m_encoder(),
  m_decoder(),
  m_debug(debug)
{
}

Headset::~Headset()
{
  finalize_wav();
  m_interface.reset();
}

void
Headset::write_wav_header(std::ostream& out, uint32_t data_bytes)
{
  const uint32_t byte_rate = WAV_SAMPLE_RATE * WAV_CHANNELS * (WAV_BITS / 8);
  const uint16_t block_align = WAV_CHANNELS * (WAV_BITS / 8);
  const uint32_t riff_size = 36 + data_bytes;

  auto write_u16 = [&](uint16_t v) {
    char b[2] = { static_cast<char>(v & 0xff), static_cast<char>((v >> 8) & 0xff) };
    out.write(b, 2);
  };
  auto write_u32 = [&](uint32_t v) {
    char b[4] = {
      static_cast<char>(v & 0xff),
      static_cast<char>((v >> 8) & 0xff),
      static_cast<char>((v >> 16) & 0xff),
      static_cast<char>((v >> 24) & 0xff)
    };
    out.write(b, 4);
  };

  out.write("RIFF", 4);
  write_u32(riff_size);
  out.write("WAVE", 4);
  out.write("fmt ", 4);
  write_u32(16); // PCM fmt chunk size
  write_u16(1);  // PCM format
  write_u16(WAV_CHANNELS);
  write_u32(WAV_SAMPLE_RATE);
  write_u32(byte_rate);
  write_u16(block_align);
  write_u16(WAV_BITS);
  out.write("data", 4);
  write_u32(data_bytes);
}

void
Headset::finalize_wav()
{
  if (!m_fout_wav)
  {
    return;
  }

  m_fout_wav->flush();
  m_fout_wav->seekp(0, std::ios::beg);
  write_wav_header(*m_fout_wav, m_wav_data_bytes);
  m_fout_wav->flush();
  m_fout_wav.reset();
}

void
Headset::load_wav_as_pcm(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
  if (!in)
  {
    raise_exception(std::runtime_error, filename << ": " << strerror(errno));
  }

  char riff[12];
  if (!in.read(riff, 12) || std::memcmp(riff, "RIFF", 4) != 0 ||
      std::memcmp(riff + 8, "WAVE", 4) != 0)
  {
    raise_exception(std::runtime_error, filename << ": not a RIFF/WAVE file");
  }

  uint16_t audio_format = 0;
  uint16_t channels = 0;
  uint32_t sample_rate = 0;
  uint16_t bits_per_sample = 0;
  std::vector<char> data_chunk;

  while (in)
  {
    char hdr[8];
    if (!in.read(hdr, 8))
    {
      break;
    }
    uint32_t chunk_size = read_u32_le(hdr + 4);
    if (std::memcmp(hdr, "fmt ", 4) == 0)
    {
      if (chunk_size < 16)
      {
        raise_exception(std::runtime_error, filename << ": fmt chunk too small");
      }
      std::vector<char> fmt(chunk_size);
      if (!in.read(fmt.data(), static_cast<std::streamsize>(chunk_size)))
      {
        raise_exception(std::runtime_error, filename << ": short fmt chunk");
      }
      audio_format    = read_u16_le(fmt.data() + 0);
      channels        = read_u16_le(fmt.data() + 2);
      sample_rate     = read_u32_le(fmt.data() + 4);
      bits_per_sample = read_u16_le(fmt.data() + 14);
      // skip pad byte if odd size
      if (chunk_size & 1)
      {
        in.ignore(1);
      }
    }
    else if (std::memcmp(hdr, "data", 4) == 0)
    {
      data_chunk.resize(chunk_size);
      if (!in.read(data_chunk.data(), static_cast<std::streamsize>(chunk_size)))
      {
        raise_exception(std::runtime_error, filename << ": short data chunk");
      }
      break;
    }
    else
    {
      in.seekg(chunk_size + (chunk_size & 1), std::ios::cur);
    }
  }

  if (audio_format != 1)
  {
    raise_exception(std::runtime_error,
                    filename << ": only uncompressed PCM (format 1) is supported");
  }
  if (bits_per_sample != 16)
  {
    raise_exception(std::runtime_error,
                    filename << ": only 16-bit PCM is supported");
  }
  if (channels < 1 || channels > 2)
  {
    raise_exception(std::runtime_error,
                    filename << ": only mono or stereo is supported");
  }
  if (sample_rate == 0 || data_chunk.empty())
  {
    raise_exception(std::runtime_error, filename << ": missing or empty audio data");
  }

  const size_t frame_bytes = static_cast<size_t>(channels) * 2;
  const size_t num_frames = data_chunk.size() / frame_bytes;

  // Downmix to mono S16LE at source rate
  std::vector<int16_t> mono(num_frames);
  for (size_t i = 0; i < num_frames; ++i)
  {
    const char* p = data_chunk.data() + i * frame_bytes;
    int16_t left = static_cast<int16_t>(read_u16_le(p));
    if (channels == 2)
    {
      int16_t right = static_cast<int16_t>(read_u16_le(p + 2));
      mono[i] = static_cast<int16_t>((static_cast<int>(left) + right) / 2);
    }
    else
    {
      mono[i] = left;
    }
  }

  // Linear resample to playback rate (8 kHz)
  if (sample_rate == PLAY_SAMPLE_RATE)
  {
    m_play_pcm = std::move(mono);
  }
  else
  {
    const double ratio = static_cast<double>(PLAY_SAMPLE_RATE) / sample_rate;
    const size_t out_frames = static_cast<size_t>(std::llround(num_frames * ratio));
    m_play_pcm.resize(out_frames);
    for (size_t i = 0; i < out_frames; ++i)
    {
      double src = i / ratio;
      size_t i0 = static_cast<size_t>(src);
      size_t i1 = std::min(i0 + 1, num_frames - 1);
      double frac = src - i0;
      double s = mono[i0] * (1.0 - frac) + mono[i1] * frac;
      m_play_pcm[i] = static_cast<int16_t>(std::lround(s));
    }
  }

  m_play_pos = 0;
  log_info("[headset] play-wav: {} frames @ {} Hz → {} frames @ {} Hz",
           num_frames, sample_rate, m_play_pcm.size(), PLAY_SAMPLE_RATE);
}

void
Headset::play_file(std::string const& filename)
{
  m_fin.reset(new std::ifstream(filename.c_str(), std::ios::binary));
  m_play_pcm.clear();
  m_play_pos = 0;

  if (!*m_fin)
  {
    std::ostringstream out;
    out << "[headset] " << filename << ": " << strerror(errno);
    throw std::runtime_error(out.str());
  }
  else
  {
    char data[32];
    int len = static_cast<int>(m_fin->read(data, sizeof(data)).gcount());
    if (len != 32)
    {
      log_error("short read");
    }
    else
    {
      m_interface->submit_write(4, reinterpret_cast<uint8_t*>(data), len,
                                std::bind(&Headset::send_data, this, _1));
    }
  }
}

void
Headset::play_wav(std::string const& filename)
{
  m_fin.reset(); // not using raw file mode
  load_wav_as_pcm(filename);

  if (m_play_pcm.size() < SAMPLES_PER_PACKET)
  {
    raise_exception(std::runtime_error, filename << ": audio too short for one packet");
  }

  // Encode first packet and kick off the interrupt OUT stream
  std::vector<uint8_t> packet;
  m_encoder.encode(m_play_pcm.data(), SAMPLES_PER_PACKET, packet);
  m_play_pos = SAMPLES_PER_PACKET;

  if (packet.size() != 32)
  {
    raise_exception(std::runtime_error, "internal: expected 32-byte G.726 packet");
  }

  m_interface->submit_write(4, packet.data(), 32,
                            std::bind(&Headset::send_data, this, _1));
}

void
Headset::record_file(std::string const& filename)
{
  m_fout_raw.reset(new std::ofstream(filename.c_str(), std::ios::binary));

  if (!*m_fout_raw)
  {
    raise_exception(std::runtime_error, filename << ": " << strerror(errno));
  }
  else
  {
    m_interface->submit_read(3, 32, std::bind(&Headset::receive_data, this, _1, _2));
  }
}

void
Headset::record_pcm(std::string const& filename)
{
  m_fout_pcm.reset(new std::ofstream(filename.c_str(), std::ios::binary));

  if (!*m_fout_pcm)
  {
    raise_exception(std::runtime_error, filename << ": " << strerror(errno));
  }
  else
  {
    m_interface->submit_read(3, 32, std::bind(&Headset::receive_data, this, _1, _2));
  }
}

void
Headset::record_wav(std::string const& filename)
{
  m_fout_wav.reset(new std::fstream(filename.c_str(),
                                    std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc));

  if (!*m_fout_wav)
  {
    raise_exception(std::runtime_error, filename << ": " << strerror(errno));
  }

  m_wav_data_bytes = 0;
  // Placeholder header; sizes rewritten in finalize_wav()
  write_wav_header(*m_fout_wav, 0);
  m_interface->submit_read(3, 32, std::bind(&Headset::receive_data, this, _1, _2));
}

bool
Headset::send_data(libusb_transfer* transfer)
{
  // Prefer in-memory PCM encode path (play-wav)
  if (!m_play_pcm.empty())
  {
    if (m_play_pos + SAMPLES_PER_PACKET > m_play_pcm.size())
    {
      log_info("[headset] play-wav finished ({} samples)", m_play_pcm.size());
      return false;
    }

    std::vector<uint8_t> packet;
    m_encoder.encode(m_play_pcm.data() + m_play_pos, SAMPLES_PER_PACKET, packet);
    m_play_pos += SAMPLES_PER_PACKET;

    if (packet.size() != 32)
    {
      log_error("[headset] encode produced {} bytes, expected 32", packet.size());
      return false;
    }
    std::memcpy(transfer->buffer, packet.data(), 32);
    return true;
  }

  // Legacy raw G.726 file path
  if (!m_fin)
  {
    return false;
  }

  int len = static_cast<int>(m_fin->read(reinterpret_cast<char*>(transfer->buffer), transfer->length).gcount());

  if (len != 32)
  {
    log_error("short read");
    return false;
  }
  else
  {
    return true;
  }
}

bool
Headset::receive_data(uint8_t* data, int len)
{
  if (m_fout_raw.get())
  {
    m_fout_raw->write(reinterpret_cast<char*>(data), len);
  }

  if ((m_fout_pcm.get() || m_fout_wav.get()) && len > 0)
  {
    std::vector<int16_t> samples;
    m_decoder.decode(data, len, samples);
    const char* bytes = reinterpret_cast<const char*>(samples.data());
    const std::streamsize nbytes = static_cast<std::streamsize>(samples.size() * sizeof(int16_t));

    if (m_fout_pcm.get())
    {
      m_fout_pcm->write(bytes, nbytes);
      m_fout_pcm->flush();
    }

    if (m_fout_wav.get())
    {
      m_fout_wav->write(bytes, nbytes);
      m_wav_data_bytes += static_cast<uint32_t>(nbytes);
    }
  }

  if (m_debug)
  {
    log_debug(raw2str(data, len));
  }

  return true;
}

} // namespace xboxdrv

/* EOF */
