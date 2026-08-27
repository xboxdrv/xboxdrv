/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
*/

#ifndef HEADER_XBOXDRV_HEADSET_HPP
#define HEADER_XBOXDRV_HEADSET_HPP

#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <libusb.h>

#include <unsebu/usb_interface.hpp>

#include "g72x_decoder.hpp"

namespace xboxdrv {

class Headset
{
private:
  libusb_device_handle* m_handle;
  std::unique_ptr<unsebu::USBInterface> m_interface;

  std::unique_ptr<std::ofstream> m_fout_raw;
  std::unique_ptr<std::ofstream> m_fout_pcm;
  std::unique_ptr<std::fstream> m_fout_wav;
  uint32_t m_wav_data_bytes;

  std::unique_ptr<std::ifstream> m_fin;
  std::vector<int16_t> m_play_pcm;
  size_t m_play_pos;
  bool m_play_left_pack;
  /** Non-blocking fd for module-pipe-sink PCM (S16LE @ 8 kHz mono). */
  int m_pulse_play_fd;
  std::vector<int16_t> m_pulse_play_carry;
  int m_pulse_source_module;
  int m_pulse_sink_module;
  std::string m_pulse_mic_fifo;
  std::string m_pulse_spk_fifo;
  G72xEncoder m_encoder;
  G72xDecoder m_decoder;
  bool m_debug;

public:
  Headset(libusb_device_handle* handle, bool debug);
  ~Headset();

  void play_file(const std::string& play_filename);
  void play_wav(const std::string& wav_filename, bool left_pack = false);
  void record_file(const std::string& dump_filename);
  void record_pcm(const std::string& pcm_filename);
  void record_wav(const std::string& wav_filename);

  /** Expose mic/speaker via pactl module-pipe-source/sink (PipeWire pulse or PulseAudio). */
  void enable_pulse_audio();

private:
  bool send_data(libusb_transfer* transfer);
  bool receive_data(uint8_t* data, int len);

  void write_wav_header(std::ostream& out, uint32_t data_bytes);
  void finalize_wav();

  void load_wav_as_pcm(const std::string& filename);
  void encode_packet(const int16_t* samples, std::vector<uint8_t>& out);

  static int pactl_load_module(const std::string& args);
  static void pactl_unload_module(int index);
  void start_pulse_playback();
  bool fill_play_samples_from_pulse(int16_t* out, size_t count);

private:
  Headset(const Headset&);
  Headset& operator=(const Headset&);
};

} // namespace xboxdrv

#endif

/* EOF */
