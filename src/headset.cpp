/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#include "headset.hpp"

#ifdef HAVE_PIPEWIRE
#include "headset_pipewire.hpp"
#endif

#include <fstream>
#include <functional>
#include <errno.h>
#include <string.h>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>

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
/** OUT transfers must be 32 bytes (16-byte packets produced silence; raw
    --headset-play uses 32 and works). At 8 kHz that is 64 samples / packet. */
constexpr size_t PLAY_PACKET_BYTES = 32;
constexpr size_t SAMPLES_PER_PACKET = PLAY_PACKET_BYTES * 2; // 64 samples 

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

Headset::Headset(libusb_device_handle* handle, bool debug, float mic_gain,
                 int usb_interface, int ep_in, int ep_out, bool try_detach) :
  m_handle(handle),
  m_interface(new unsebu::USBInterface(m_handle, usb_interface, try_detach)),
  m_fout_raw(),
  m_fout_pcm(),
  m_fout_wav(),
  m_wav_data_bytes(0),
  m_fin(),
  m_play_pcm(),
  m_play_pos(0),
  m_play_left_pack(false),
  m_pulse_play_fd(-1),
  m_pulse_play_carry(),
  m_pulse_source_module(-1),
  m_pulse_sink_module(-1),
  m_pulse_mic_fifo(),
  m_pulse_spk_fifo(),
  m_encoder(),
  m_decoder(),
  m_mic_gain(mic_gain),
  m_ep_in(ep_in),
  m_ep_out(ep_out),
  m_debug(debug)
{
  log_info("[headset] interface {} EP IN {} OUT {}", usb_interface, m_ep_in, m_ep_out);
  if (m_mic_gain != 1.0f)
  {
    log_info("[headset] mic gain {}", m_mic_gain);
  }
}

Headset::~Headset()
{
  finalize_wav();
#ifdef HAVE_PIPEWIRE
  if (m_pw)
  {
    m_pw->shutdown();
    m_pw.reset();
  }
#endif
  if (m_pulse_play_fd >= 0)
  {
    ::close(m_pulse_play_fd);
    m_pulse_play_fd = -1;
  }
  if (m_pulse_sink_module >= 0)
  {
    pactl_unload_module(m_pulse_sink_module);
    m_pulse_sink_module = -1;
  }
  if (m_pulse_source_module >= 0)
  {
    pactl_unload_module(m_pulse_source_module);
    m_pulse_source_module = -1;
  }
  if (!m_pulse_mic_fifo.empty())
  {
    ::unlink(m_pulse_mic_fifo.c_str());
  }
  if (!m_pulse_spk_fifo.empty())
  {
    ::unlink(m_pulse_spk_fifo.c_str());
  }
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

  // Mild attenuation avoids ADPCM overload crackle on peaks
  for (auto& s : m_play_pcm)
  {
    s = static_cast<int16_t>((static_cast<int>(s) * 7) / 8);
  }

  // ~100 ms of silence so the predictor settles before speech
  const size_t prime = static_cast<size_t>(PLAY_SAMPLE_RATE / 10);
  std::vector<int16_t> primed(prime + m_play_pcm.size(), 0);
  std::copy(m_play_pcm.begin(), m_play_pcm.end(), primed.begin() + static_cast<std::ptrdiff_t>(prime));
  m_play_pcm = std::move(primed);

  m_play_pos = 0;
  log_info("[headset] play-wav: {} frames @ {} Hz → {} frames @ {} Hz (incl. {} silence prime)",
           num_frames, sample_rate, m_play_pcm.size(), PLAY_SAMPLE_RATE, prime);
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
      m_interface->submit_write(m_ep_out, reinterpret_cast<uint8_t*>(data), len,
                                std::bind(&Headset::send_data, this, _1));
    }
  }
}

void
Headset::encode_packet(const int16_t* samples, std::vector<uint8_t>& out)
{
  m_encoder.encode(samples, SAMPLES_PER_PACKET, out, m_play_left_pack);
}

void
Headset::play_wav(std::string const& filename, bool left_pack)
{
  m_fin.reset(); // not using raw file mode
  m_play_left_pack = left_pack;
  m_encoder.reset();
  load_wav_as_pcm(filename);

  if (m_play_pcm.size() < SAMPLES_PER_PACKET)
  {
    raise_exception(std::runtime_error, filename << ": audio too short for one packet");
  }

  log_info("[headset] play-wav packing: {}; packet {} bytes / {} samples @ {} Hz",
           left_pack ? "left (high nibble first)" : "right (low nibble first)",
           PLAY_PACKET_BYTES, SAMPLES_PER_PACKET, PLAY_SAMPLE_RATE);

  // Encode first packet and kick off the interrupt OUT stream
  std::vector<uint8_t> packet;
  encode_packet(m_play_pcm.data(), packet);
  m_play_pos = SAMPLES_PER_PACKET;

  if (packet.size() != PLAY_PACKET_BYTES)
  {
    raise_exception(std::runtime_error,
                    "internal: expected " << PLAY_PACKET_BYTES << "-byte G.726 packet");
  }

  m_interface->submit_write(m_ep_out, packet.data(), static_cast<int>(PLAY_PACKET_BYTES),
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
    m_interface->submit_read(m_ep_in, 32, std::bind(&Headset::receive_data, this, _1, _2));
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
    m_interface->submit_read(m_ep_in, 32, std::bind(&Headset::receive_data, this, _1, _2));
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
  m_interface->submit_read(m_ep_in, 32, std::bind(&Headset::receive_data, this, _1, _2));
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
    encode_packet(m_play_pcm.data() + m_play_pos, packet);
    m_play_pos += SAMPLES_PER_PACKET;

    if (packet.size() != PLAY_PACKET_BYTES)
    {
      log_error("[headset] encode produced {} bytes, expected {}",
                packet.size(), PLAY_PACKET_BYTES);
      return false;
    }
    if (static_cast<size_t>(transfer->length) < PLAY_PACKET_BYTES)
    {
      log_error("[headset] transfer buffer too small ({})", transfer->length);
      return false;
    }
    std::memcpy(transfer->buffer, packet.data(), PLAY_PACKET_BYTES);
    // Keep transfer length at play packet size (initial submit sets it).
    transfer->length = static_cast<int>(PLAY_PACKET_BYTES);
    return true;
  }

#ifdef HAVE_PIPEWIRE
  if (m_pw && m_pw->running())
  {
    int16_t samples[SAMPLES_PER_PACKET];
    if (!m_pw->pull_speaker(samples, SAMPLES_PER_PACKET))
    {
      std::memset(samples, 0, sizeof(samples));
    }
    std::vector<uint8_t> packet;
    encode_packet(samples, packet);
    if (packet.size() != PLAY_PACKET_BYTES)
    {
      return false;
    }
    std::memcpy(transfer->buffer, packet.data(), PLAY_PACKET_BYTES);
    transfer->length = static_cast<int>(PLAY_PACKET_BYTES);
    return true;
  }
#endif

  // module-pipe-sink PCM → encode → OUT
  if (m_pulse_play_fd >= 0)
  {
    int16_t samples[SAMPLES_PER_PACKET];
    if (!fill_play_samples_from_pulse(samples, SAMPLES_PER_PACKET))
    {
      std::memset(samples, 0, sizeof(samples));
    }
    std::vector<uint8_t> packet;
    encode_packet(samples, packet);
    if (packet.size() != PLAY_PACKET_BYTES)
    {
      return false;
    }
    std::memcpy(transfer->buffer, packet.data(), PLAY_PACKET_BYTES);
    transfer->length = static_cast<int>(PLAY_PACKET_BYTES);
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

  bool need_decode = m_fout_pcm.get() || m_fout_wav.get();
#ifdef HAVE_PIPEWIRE
  need_decode = need_decode || (m_pw && m_pw->running());
#endif
  if (need_decode && len > 0)
  {
    std::vector<int16_t> samples;
    m_decoder.decode(data, len, samples);
    if (m_mic_gain != 1.0f)
    {
      for (auto& s : samples)
      {
        int v = static_cast<int>(std::lround(static_cast<float>(s) * m_mic_gain));
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        s = static_cast<int16_t>(v);
      }
    }
    const char* bytes = reinterpret_cast<const char*>(samples.data());
    const std::streamsize nbytes = static_cast<std::streamsize>(samples.size() * sizeof(int16_t));

    if (m_fout_pcm.get())
    {
      m_fout_pcm->write(bytes, nbytes);
      m_fout_pcm->flush();
    }
#ifdef HAVE_PIPEWIRE
    if (m_pw && m_pw->running())
    {
      m_pw->push_mic(samples.data(), samples.size());
    }
#endif
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




int
Headset::pactl_load_module(std::string const& args)
{
  std::string cmd = "pactl load-module " + args + " 2>/dev/null";
  FILE* fp = ::popen(cmd.c_str(), "r");
  if (!fp)
  {
    return -1;
  }
  char buf[64];
  std::string out;
  while (std::fgets(buf, sizeof(buf), fp))
  {
    out += buf;
  }
  int status = ::pclose(fp);
  if (status != 0)
  {
    return -1;
  }
  try
  {
    return std::stoi(out);
  }
  catch (...)
  {
    return -1;
  }
}

void
Headset::pactl_unload_module(int index)
{
  if (index < 0)
  {
    return;
  }
  std::string cmd = "pactl unload-module " + std::to_string(index) + " 2>/dev/null";
  int r = ::system(cmd.c_str());
  (void)r;
}

bool
Headset::fill_play_samples_from_pulse(int16_t* out, size_t count)
{
  size_t filled = 0;
  if (!m_pulse_play_carry.empty())
  {
    size_t n = std::min(m_pulse_play_carry.size(), count);
    std::memcpy(out, m_pulse_play_carry.data(), n * sizeof(int16_t));
    m_pulse_play_carry.erase(m_pulse_play_carry.begin(),
                             m_pulse_play_carry.begin() + static_cast<std::ptrdiff_t>(n));
    filled = n;
  }
  while (filled < count && m_pulse_play_fd >= 0)
  {
    uint8_t buf[512];
    ssize_t n = ::read(m_pulse_play_fd, buf, sizeof(buf));
    if (n < 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        break;
      }
      log_warn("[headset] pulse play read: {}", strerror(errno));
      break;
    }
    if (n == 0)
    {
      break;
    }
    size_t nsamp = static_cast<size_t>(n) / sizeof(int16_t);
    const int16_t* s = reinterpret_cast<const int16_t*>(buf);
    for (size_t i = 0; i < nsamp; ++i)
    {
      if (filled < count)
      {
        out[filled++] = s[i];
      }
      else
      {
        m_pulse_play_carry.push_back(s[i]);
      }
    }
  }
  // Bound carry so pipe backlog cannot grow without limit (~100 ms @ 8 kHz).
  constexpr size_t kMaxCarry = 800;
  if (m_pulse_play_carry.size() > kMaxCarry)
  {
    m_pulse_play_carry.erase(m_pulse_play_carry.begin(),
                             m_pulse_play_carry.end() - static_cast<std::ptrdiff_t>(kMaxCarry));
  }
  if (filled < count)
  {
    std::memset(out + filled, 0, (count - filled) * sizeof(int16_t));
    return filled > 0;
  }
  return true;
}

void
Headset::start_pulse_playback()
{
  m_encoder.reset();
  m_play_left_pack = false;
  m_play_pcm.clear();
  m_play_pos = 0;
  m_fin.reset();

  int16_t silence[SAMPLES_PER_PACKET];
  std::memset(silence, 0, sizeof(silence));
  std::vector<uint8_t> packet;
  encode_packet(silence, packet);
  if (packet.size() != PLAY_PACKET_BYTES)
  {
    log_error("[headset] pulse: bad silence packet");
    return;
  }
  m_interface->submit_write(m_ep_out, packet.data(), static_cast<int>(PLAY_PACKET_BYTES),
                            std::bind(&Headset::send_data, this, _1));
  log_info("[headset] speaker USB stream started (8 kHz from pipe-sink)");
}

void
Headset::enable_pulse_audio()
{
  const char* runtime = std::getenv("XDG_RUNTIME_DIR");
  std::string dir = runtime ? std::string(runtime) + "/xboxdrv" : "/tmp/xboxdrv";
  if (::mkdir(dir.c_str(), 0700) != 0 && errno != EEXIST)
  {
    raise_exception(std::runtime_error, "mkdir " << dir << ": " << strerror(errno));
  }

  m_pulse_mic_fifo = dir + "/mic.pcm";
  m_pulse_spk_fifo = dir + "/speaker.pcm";
  ::unlink(m_pulse_mic_fifo.c_str());
  ::unlink(m_pulse_spk_fifo.c_str());
  if (::mkfifo(m_pulse_mic_fifo.c_str(), 0600) != 0)
  {
    raise_exception(std::runtime_error, "mkfifo " << m_pulse_mic_fifo << ": " << strerror(errno));
  }
  if (::mkfifo(m_pulse_spk_fifo.c_str(), 0600) != 0)
  {
    raise_exception(std::runtime_error, "mkfifo " << m_pulse_spk_fifo << ": " << strerror(errno));
  }

  {
    std::string args = "module-pipe-source source_name=xboxdrv-headset-mic "
      "file=" + m_pulse_mic_fifo + " format=s16le rate=16000 channels=1";
    m_pulse_source_module = pactl_load_module(args);
    if (m_pulse_source_module < 0)
    {
      raise_exception(std::runtime_error,
                      "pactl load-module module-pipe-source failed "
                      "(is PulseAudio or PipeWire-pulse running? is pactl on PATH?)");
    }
    log_info("[headset] pulse source 'xboxdrv-headset-mic' (module {}) <- {}",
             m_pulse_source_module, m_pulse_mic_fifo);
    record_pcm(m_pulse_mic_fifo);
  }

  {
    std::string args = "module-pipe-sink sink_name=xboxdrv-headset-speaker "
      "file=" + m_pulse_spk_fifo + " format=s16le rate=8000 channels=1";
    m_pulse_sink_module = pactl_load_module(args);
    if (m_pulse_sink_module < 0)
    {
      log_warn("[headset] pactl load-module module-pipe-sink failed; mic source only");
    }
    else
    {
      log_info("[headset] pulse sink 'xboxdrv-headset-speaker' (module {}) -> {}",
               m_pulse_sink_module, m_pulse_spk_fifo);
      // O_RDWR so open does not block waiting for a writer; non-blocking reads.
      m_pulse_play_fd = ::open(m_pulse_spk_fifo.c_str(), O_RDWR | O_NONBLOCK);
      if (m_pulse_play_fd < 0)
      {
        log_warn("[headset] open {}: {}", m_pulse_spk_fifo, strerror(errno));
      }
      else
      {
        // Limit kernel pipe buffer (~64 KiB default can add a lot of latency).
#ifdef F_SETPIPE_SZ
        (void)::fcntl(m_pulse_play_fd, F_SETPIPE_SZ, 4096);
#endif
        start_pulse_playback();
      }
    }
  }
}


void
Headset::enable_pipewire_audio()
{
#ifdef HAVE_PIPEWIRE
  m_pw = std::make_unique<HeadsetPipeWire>();
  m_pw->start();
  if (!m_fout_pcm && !m_fout_raw && !m_fout_wav)
  {
    m_interface->submit_read(m_ep_in, 32, std::bind(&Headset::receive_data, this, _1, _2));
  }
  start_pipewire_playback();
#else
  raise_exception(std::runtime_error,
                  "--headset-pipewire requires xboxdrv built with libpipewire-0.3 "
                  "(HAVE_PIPEWIRE)");
#endif
}

#ifdef HAVE_PIPEWIRE
void
Headset::start_pipewire_playback()
{
  m_encoder.reset();
  m_play_left_pack = false;
  m_play_pcm.clear();
  m_play_pos = 0;
  m_fin.reset();

  int16_t silence[SAMPLES_PER_PACKET];
  std::memset(silence, 0, sizeof(silence));
  std::vector<uint8_t> packet;
  encode_packet(silence, packet);
  if (packet.size() != PLAY_PACKET_BYTES)
  {
    log_error("[headset] pipewire: bad silence packet");
    return;
  }
  m_interface->submit_write(m_ep_out, packet.data(), static_cast<int>(PLAY_PACKET_BYTES),
                            std::bind(&Headset::send_data, this, _1));
  log_info("[headset] speaker USB stream started (8 kHz from PipeWire sink)");
}
#endif

} // namespace xboxdrv

/* EOF */
