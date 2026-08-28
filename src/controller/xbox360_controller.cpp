/*
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008-2026 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "controller/xbox360_controller.hpp"

#include <unsebu/usb_helper.hpp>

#include "chatpad.hpp"
#include "headset.hpp"
#include "util/string.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "unpack.hpp"

namespace xboxdrv {

Xbox360Controller::Xbox360Controller(libusb_device* dev,
                                     bool chatpad, bool chatpad_no_init, bool chatpad_debug,
                                     bool headset,
                                     bool headset_debug,
                                     std::string const& headset_dump,
                                     std::string const& headset_play,
                                     std::string const& headset_pcm,
                                     std::string const& headset_wav,
                                     std::string const& headset_play_wav,
                                     bool headset_play_left_pack,
                                     bool headset_pulse,
                                     bool headset_pipewire,
                                     float headset_mic_gain,
                                     bool try_detach) :
  USBController(dev),
  dev_type(),
  endpoint_in(1),
  endpoint_out(2),
  m_chatpad(),
  m_headset(),
  m_rumble_left(0),
  m_rumble_right(0),
  xbox(m_message_descriptor)
{
  // find endpoints
  endpoint_in  = usb_find_ep(LIBUSB_ENDPOINT_IN,  LIBUSB_CLASS_VENDOR_SPEC, 93, 1);
  endpoint_out = usb_find_ep(LIBUSB_ENDPOINT_OUT, LIBUSB_CLASS_VENDOR_SPEC, 93, 1);

  log_debug("EP(IN):  {}", endpoint_in);
  log_debug("EP(OUT): {}", endpoint_out);

  usb_claim_interface(0, try_detach);
  usb_submit_read(endpoint_in, 32);

  // create chatpad
  if (chatpad)
  {
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << libusb_strerror(ret));
    }
    else
    {
      m_chatpad.reset(new Chatpad(m_handle, desc.bcdDevice, chatpad_no_init, chatpad_debug));
    }
  }

  // create headset
  if (headset)
  {
    m_headset.reset(new Headset(m_handle, headset_debug, headset_mic_gain));
    if (!headset_play.empty())
    {
      m_headset->play_file(headset_play);
    }

    if (!headset_dump.empty())
    {
      m_headset->record_file(headset_dump);
    }

    if (!headset_pcm.empty())
    {
      m_headset->record_pcm(headset_pcm);
    }

    if (!headset_wav.empty())
    {
      m_headset->record_wav(headset_wav);
    }

    if (!headset_play_wav.empty())
    {
      m_headset->play_wav(headset_play_wav, headset_play_left_pack);
    }

    if (headset_pulse)
    {
      m_headset->enable_pulse_audio();
    }
    if (headset_pipewire)
    {
      m_headset->enable_pipewire_audio();
    }
  }
}

Xbox360Controller::~Xbox360Controller()
{
  // Drop headset (and PipeWire nodes) before other USB teardown via members.
  m_headset.reset();
  m_chatpad.reset();
}

void
Xbox360Controller::set_rumble_real(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  usb_write(endpoint_out, rumblecmd, sizeof(rumblecmd));
}

void
Xbox360Controller::set_led_real(uint8_t status)
{
  uint8_t ledcmd[] = { 0x01, 0x03, status };
  usb_write(endpoint_out, ledcmd, sizeof(ledcmd));
}

bool
Xbox360Controller::parse(uint8_t const* data, int len, ControllerMessage* msg_out)
{
  if (len == 0)
  {
    // happens with the Xbox360 controller every now and then, just
    // ignore, seems harmless, so just ignore
    //log_debug("zero length read");
  }
  else if (len == 3 && data[0] == 0x01 && data[1] == 0x03)
  {
    log_debug("Xbox360Controller: LED Status: {}", int(data[2]));
  }
  else if (len == 3 && data[0] == 0x03 && data[1] == 0x03)
  {
    // data[2] == 0x00 means that rumble is disabled
    // data[2] == 0x01 unknown, but rumble works
    // data[2] == 0x02 unknown, but rumble works
    // data[2] == 0x03 is default with rumble enabled
    log_info("rumble status: {}", int(data[2]));
  }
  else if (len == 3 && data[0] == 0x08 && data[1] == 0x03)
  {
    // FIXME: maybe a proper indicator for the actvity on the chatpad
    // port, so that we don't have to send chatpad init
    if (data[2] == 0x00)
    {
      log_info("peripheral: none");
    }
    else if (data[2] == 0x01)
    {
      log_info("peripheral: chatpad");
    }
    else if (data[2] == 0x02)
    {
      log_info("peripheral: headset");
    }
    else if (data[2] == 0x03)
    {
      log_info("peripheral: headset, chatpad");
    }
    else
    {
      log_info("peripheral: unknown: {}", int(data[2]));
    }
  }
  else if (len >= 18 && data[0] == 0x00)
  {
    // Wired 360 input report: type byte 0x00. Official pads use length 0x14
    // (20 bytes); kernel xpad only checks data[0]==0, so accept len>=18 for
    // clones that omit or alter the length byte.

    msg_out->set_key(xbox.dpad_up,    unpack::bit(data+2, 0));
    msg_out->set_key(xbox.dpad_down,  unpack::bit(data+2, 1));
    msg_out->set_key(xbox.dpad_left,  unpack::bit(data+2, 2));
    msg_out->set_key(xbox.dpad_right, unpack::bit(data+2, 3));

    msg_out->set_key(xbox.btn_start,   unpack::bit(data+2, 4));
    msg_out->set_key(xbox.btn_back,    unpack::bit(data+2, 5));
    msg_out->set_key(xbox.btn_thumb_l, unpack::bit(data+2, 6));
    msg_out->set_key(xbox.btn_thumb_r, unpack::bit(data+2, 7));

    msg_out->set_key(xbox.btn_lb, unpack::bit(data+3, 0));
    msg_out->set_key(xbox.btn_rb, unpack::bit(data+3, 1));
    msg_out->set_key(xbox.btn_guide, unpack::bit(data+3, 2));
    //msg_out->dummy1 = unpack::bit(data+3, 3);

    msg_out->set_key(xbox.btn_a, unpack::bit(data+3, 4));
    msg_out->set_key(xbox.btn_b, unpack::bit(data+3, 5));
    msg_out->set_key(xbox.btn_x, unpack::bit(data+3, 6));
    msg_out->set_key(xbox.btn_y, unpack::bit(data+3, 7));

    msg_out->set_abs(xbox.abs_lt, data[4], 0, 255);
    msg_out->set_abs(xbox.abs_rt, data[5], 0, 255);

    msg_out->set_abs(xbox.abs_x1, unpack::int16le(data+6), -32768, 32767);
    msg_out->set_abs(xbox.abs_y1, unpack::s16_invert(unpack::int16le(data+8)), -32768, 32767);

    msg_out->set_abs(xbox.abs_x2, unpack::int16le(data+10), -32768, 32767);
    msg_out->set_abs(xbox.abs_y2, unpack::s16_invert(unpack::int16le(data+12)), -32768, 32767);

    //msg.dummy2 = unpack::int32le(data+14);
    //msg.dummy3 = unpack::int16le(data+18);

    return true;
  }
  else
  {
    log_debug("unknown: {}", raw2str(data, len));
  }

  return false;
}

} // namespace xboxdrv

/* EOF */
