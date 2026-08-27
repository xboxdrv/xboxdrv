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

#include <unsebu/usb_helper.hpp>

#include "raise_exception.hpp"
#include "util/string.hpp"
#include <logmich/log.hpp>

namespace xboxdrv {

using namespace std::placeholders;

namespace {

constexpr uint32_t WAV_SAMPLE_RATE = 16000;
constexpr uint16_t WAV_CHANNELS = 1;
constexpr uint16_t WAV_BITS = 16;

} // namespace

Headset::Headset(libusb_device_handle* handle, bool debug) :
  m_handle(handle),
  m_interface(new unsebu::USBInterface(m_handle, 1)),
  m_fout_raw(),
  m_fout_pcm(),
  m_fout_wav(),
  m_wav_data_bytes(0),
  m_fin(),
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
Headset::play_file(std::string const& filename)
{
  m_fin.reset(new std::ifstream(filename.c_str(), std::ios::binary));

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
