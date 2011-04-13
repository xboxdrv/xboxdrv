/* 
**  Xbox/Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xbox360_controller.hpp"

#include "chatpad.hpp"
#include "headset.hpp"
#include "helper.hpp"
#include "options.hpp"
#include "raise_exception.hpp"
#include "usb_helper.hpp"

Xbox360Controller::Xbox360Controller(libusb_device* dev,
                                     bool chatpad, bool chatpad_no_init, bool chatpad_debug, 
                                     bool headset, 
                                     bool headset_debug, 
                                     const std::string& headset_dump,
                                     const std::string& headset_play,
                                     bool try_detach) :
  USBController(dev),
  dev_type(),
  endpoint_in(1),
  endpoint_out(2),
  m_chatpad(),
  m_headset()
{
  // find endpoints
  endpoint_in  = usb_find_ep(LIBUSB_ENDPOINT_IN,  LIBUSB_CLASS_VENDOR_SPEC, 93, 1);
  endpoint_out = usb_find_ep(LIBUSB_ENDPOINT_OUT, LIBUSB_CLASS_VENDOR_SPEC, 93, 1);

  log_debug("EP(IN):  " << endpoint_in);
  log_debug("EP(OUT): " << endpoint_out);

  usb_claim_interface(0, try_detach);
  usb_submit_read(endpoint_in, 32);

#ifdef FIXME
  // create chatpad
  if (chatpad)
  {
    libusb_device_descriptor desc;

    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret != LIBUSB_SUCCESS)
    {
      raise_exception(std::runtime_error, "libusb_get_config_descriptor() failed: " << usb_strerror(ret));    }
    else
    {
      m_chatpad.reset(new Chatpad(m_handle, desc.bcdDevice, chatpad_no_init, chatpad_debug));
      m_chatpad->send_init();
      m_chatpad->start_threads();
    }
  }
#endif

  // create headset
  if (headset)
  {
    m_headset.reset(new Headset(m_handle, headset_debug));
    if (!headset_play.empty())
    {
      m_headset->play_file(headset_play);
    }

    if (!headset_dump.empty())
    {
      m_headset->record_file(headset_dump);
    }
  }
}

Xbox360Controller::~Xbox360Controller()
{
  usb_cancel_read();
  usb_release_interface(0);
}

void
Xbox360Controller::set_rumble(uint8_t left, uint8_t right)
{
  uint8_t rumblecmd[] = { 0x00, 0x08, 0x00, left, right, 0x00, 0x00, 0x00 };
  usb_write(endpoint_out, rumblecmd, sizeof(rumblecmd));
}

void
Xbox360Controller::set_led(uint8_t status)
{
  uint8_t ledcmd[] = { 0x01, 0x03, status }; 
  usb_write(endpoint_out, ledcmd, sizeof(ledcmd));
}

bool
Xbox360Controller::parse(uint8_t* data, int len, XboxGenericMsg* msg_out)
{
  if (len == 0)
  {
    // happens with the Xbox360 controller every now and then, just
    // ignore, seems harmless, so just ignore
    //log_debug("zero length read");
  }
  else if (len == 3 && data[0] == 0x01 && data[1] == 0x03)
  { 
    log_debug("Xbox360Controller: LED Status: " << int(data[2]));
  }
  else if (len == 3 && data[0] == 0x03 && data[1] == 0x03)
  { 
    // data[2] == 0x00 means that rumble is disabled
    // data[2] == 0x01 unknown, but rumble works
    // data[2] == 0x02 unknown, but rumble works
    // data[2] == 0x03 is default with rumble enabled
    log_info("rumble status: " << int(data[2]));
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
      log_info("peripheral: unknown: " << int(data[2]));
    }
  }
  else if (len == 20 && data[0] == 0x00 && data[1] == 0x14)
  {
    msg_out->type    = XBOX_MSG_XBOX360;
    memcpy(&msg_out->xbox360, data, sizeof(Xbox360Msg));
    return true;
  }
  else
  {
    log_debug("unknown: " << raw2str(data, len));
  }

  return false;
}

/* EOF */
